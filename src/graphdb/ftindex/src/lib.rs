use std::borrow::Cow;
use std::collections::HashMap;
use std::error::Error;
use std::fs;
use std::path::Path;
use std::sync::{Mutex, OnceLock};
use tantivy::{Index, IndexReader, IndexWriter, ReloadPolicy, TantivyDocument, Term};
use tantivy::collector::TopDocs;
use tantivy::directory::MmapDirectory;
use tantivy::query::QueryParser;
use tantivy::schema::{
    FAST, Field, INDEXED, IndexRecordOption, NumericOptions, STORED, Schema, TextFieldIndexing,
    TextOptions,
};
use tantivy::tokenizer::{Language, LowerCaser, RemoveLongFilter, StopWordFilter, TextAnalyzer};
use tantivy_jieba::JiebaTokenizer;
use crate::ffi::IdScore;
use crate::ffi::QueryOptions;

const ZH_TOKENIZER_NAME: &str = "jieba";
const ZH_STOP_WORDS: &str = include_str!(concat!(env!("CARGO_MANIFEST_DIR"), "/stopwords_zh.txt"));

pub struct FTIndex {
    index : Index,
    writer: Mutex<IndexWriter>,
    reader: IndexReader,
    id_field: Field,
    field_by_name: HashMap<String, Field>,
    query_parser: QueryParser,
}

fn is_ignored_char(c: char) -> bool {
    matches!(c, '\u{200B}' | '\u{200C}' | '\u{200D}' | '\u{2060}' | '\u{FEFF}')
}

fn normalize_text(text: &str) -> Cow<'_, str> {
    if !text.chars().any(is_ignored_char) {
        return Cow::Borrowed(text);
    }
    Cow::Owned(text.chars().filter(|c| !is_ignored_char(*c)).collect())
}

fn zh_stop_words() -> &'static Vec<String> {
    static STOP_WORDS: OnceLock<Vec<String>> = OnceLock::new();
    STOP_WORDS.get_or_init(|| {
        ZH_STOP_WORDS
            .split_whitespace()
            .map(std::string::ToString::to_string)
            .collect()
    })
}

fn build_analyzer() -> TextAnalyzer {
    let mut tokenizer = JiebaTokenizer::new();
    // Tantivy phrase queries expect ordinal token positions instead of byte offsets.
    tokenizer.set_ordinal_position_mode(true);
    TextAnalyzer::builder(tokenizer)
        .filter(RemoveLongFilter::limit(40))
        .filter(LowerCaser)
        .filter(StopWordFilter::remove(zh_stop_words().iter().cloned()))
        .filter(StopWordFilter::new(Language::English).expect("english stop words should exist"))
        .build()
}

fn register_tokenizer(index: &Index) {
    index.tokenizers().register(ZH_TOKENIZER_NAME, build_analyzer());
}

fn fulltext_options() -> TextOptions {
    TextOptions::default().set_indexing_options(
        TextFieldIndexing::default()
            .set_tokenizer(ZH_TOKENIZER_NAME)
            .set_index_option(IndexRecordOption::WithFreqsAndPositions),
    )
}

#[cxx::bridge]
mod ffi {
    struct IdScore {
        id: i64,
        score: f32,
    }
    struct QueryOptions {
        top_n: usize
    }
    extern "Rust" {
        type FTIndex;
        fn new_ftindex(path: &String, properties: &Vec<String>) -> Result<Box<FTIndex>>;
        fn ft_add_document(ft: &FTIndex, id:i64, fields: &Vec<String>, valus: &Vec<String>) -> Result<()>;
        fn ft_delete_document(ft: &FTIndex, id:i64) -> Result<()>;
        fn ft_commit(ft: &FTIndex, payload: &String) -> Result<()>;
        fn ft_query(ft: &FTIndex, query: &String, options: &QueryOptions) -> Result<Vec<IdScore>>;
        fn ft_get_payload(ft: &FTIndex) -> Result<String>;
        fn ft_tokenize(text: &String) -> Result<Vec<String>>;
    }
}

pub fn new_ftindex(path: &String, properties: &Vec<String>) -> Result<Box<FTIndex>, Box<dyn Error>> {
    let p = Path::new(path);
    fs::create_dir_all(&p)?;
    let mut schema_builder = Schema::builder();
    let id_field = schema_builder.add_i64_field("id", NumericOptions::default() | STORED | INDEXED | FAST);
    let mut fields: Vec<Field> = Vec::new();
    let mut field_by_name = HashMap::with_capacity(properties.len());
    for property in properties {
        let f = schema_builder.add_text_field(property, fulltext_options());
        fields.push(f);
        field_by_name.insert(property.clone(), f);
    }
    let schema = schema_builder.build();
    let mmap_directory = MmapDirectory::open(path)?;
    let index = Index::open_or_create(mmap_directory,  schema.clone())?;
    register_tokenizer(&index);
    let query_parser = QueryParser::for_index(&index, fields);
    let writer = index.writer(50_000_000)?;
    let reader = index
        .reader_builder()
        .reload_policy(ReloadPolicy::Manual)
        .try_into()?;
    let ft = FTIndex {
        index: index,
        writer: Mutex::new(writer),
        reader: reader,
        id_field: id_field,
        field_by_name: field_by_name,
        query_parser: query_parser,
    };
    return Ok(Box::new(ft));
}

pub fn ft_add_document(ft: &FTIndex, id:i64, fields: &Vec<String>, valus: &Vec<String>) -> Result<(), Box<dyn Error>> {
    let mut document = TantivyDocument::default();
    document.add_i64(ft.id_field, id);
    for i in 0..fields.len() {
        let field = *ft.field_by_name.get(fields[i].as_str()).ok_or_else(|| {
            std::io::Error::new(
                std::io::ErrorKind::InvalidInput,
                format!("unknown fulltext field: {}", fields[i]),
            )
        })?;
        let normalized = normalize_text(&valus[i]);
        document.add_text(field, normalized.as_ref());
    }
    let writer = ft.writer.lock().unwrap();
    writer.add_document(document)?;
    Ok(())
}

pub fn ft_delete_document(ft: &FTIndex, id:i64) -> Result<(), Box<dyn Error>> {
    let term = Term::from_field_i64(ft.id_field, id);
    let writer = ft.writer.lock().unwrap();
    writer.delete_term(term);
    Ok(())
}

pub fn ft_commit(ft: &FTIndex, payload: &String) -> Result<(),  Box<dyn Error>> {
    let mut writer = ft.writer.lock().unwrap();
    let mut prepared_commit = writer.prepare_commit()?;
    prepared_commit.set_payload(payload);
    prepared_commit.commit()?;
    drop(writer);
    ft.reader.reload()?;
    Ok(())
}

pub fn ft_get_payload(ft: &FTIndex) -> Result<String,  Box<dyn Error>> {
    let metas = ft.index.load_metas()?;
    Ok(metas.payload.unwrap_or("".to_string()))
}

pub fn ft_tokenize(text: &String) -> Result<Vec<String>, Box<dyn Error>> {
    let normalized = normalize_text(text);
    let mut analyzer = build_analyzer();
    let mut token_stream = analyzer.token_stream(normalized.as_ref());
    let mut tokens = Vec::new();
    while let Some(token) = token_stream.next() {
        tokens.push(token.text.clone());
    }
    Ok(tokens)
}

pub fn ft_query(ft: &FTIndex, query: &String, options: &QueryOptions) -> Result<Vec<IdScore>, Box<dyn Error>> {
    let searcher = ft.reader.searcher();
    let normalized = normalize_text(query);
    let query = ft.query_parser.parse_query(normalized.as_ref())?;
    let top_docs = searcher.search(&query, &TopDocs::with_limit(options.top_n))?;
    let id_readers = searcher
        .segment_readers()
        .iter()
        .map(|segment_reader| segment_reader.fast_fields().i64("id"))
        .collect::<Result<Vec<_>, _>>()?;
    let mut id_score: Vec<IdScore> = Vec::with_capacity(top_docs.len());
    for (score, doc_address) in top_docs {
        let id = id_readers[doc_address.segment_ord as usize].first(doc_address.doc_id).ok_or_else(
            || {
                std::io::Error::new(
                    std::io::ErrorKind::InvalidData,
                    "fulltext document missing id fast field",
                )
            },
        )?;
        let res = crate::IdScore {
            id: id,
            score: score,
        };
        id_score.push(res);
    }
    Ok(id_score)
}
