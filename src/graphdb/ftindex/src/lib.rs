use std::error::Error;
use std::path::Path;
use std::fs;
use tantivy::{Index, IndexReader, IndexWriter, ReloadPolicy, TantivyDocument, Term};
use tantivy::collector::TopDocs;
use tantivy::directory::MmapDirectory;
use tantivy::query::QueryParser;
use tantivy::schema::{FAST, Field, INDEXED, NumericOptions, Schema, STORED, TEXT, Value};
use crate::ffi::IdScore;
use crate::ffi::QueryOptions;

pub struct FTIndex {
    schema: Schema,
    index : Index,
    writer: IndexWriter,
    reader: IndexReader,
    id_field: Field,
    fields: Vec<Field>,
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
        fn ft_add_document(ft: &mut FTIndex, id:i64, fields: &Vec<String>, valus: &Vec<String>) -> Result<()>;
        fn ft_delete_document(ft: &mut FTIndex, id:i64) -> Result<()>;
        fn ft_commit(ft: &mut FTIndex) -> Result<()>;
        fn ft_query(ft: &mut FTIndex, query: &String, options: &QueryOptions) -> Result<Vec<IdScore>>;
    }
}

pub fn new_ftindex(path: &String, properties: &Vec<String>) -> Result<Box<FTIndex>, Box<dyn Error>> {
    let p = Path::new(path);
    fs::create_dir_all(&p)?;
    let mut schema_builder = Schema::builder();
    let id_field = schema_builder.add_i64_field("id", NumericOptions::default() | STORED | INDEXED | FAST);
    let mut fields: Vec<Field> = Vec::new();
    for property in properties {
        let f = schema_builder.add_text_field(property, TEXT);
        fields.push(f);
    }
    let schema = schema_builder.build();
    let mmap_directory = MmapDirectory::open(path)?;
    let index = Index::open_or_create(mmap_directory,  schema.clone())?;
    let writer = index.writer(50_000_000)?;
    let reader = index.reader_builder().reload_policy(ReloadPolicy::OnCommitWithDelay).try_into()?;
    let ft = FTIndex {
        schema: schema,
        index: index,
        writer:writer,
        reader: reader,
        id_field: id_field,
        fields: fields,
    };
    return Ok(Box::new(ft));
}

pub fn ft_add_document(ft: &mut FTIndex, id:i64, fields: &Vec<String>, valus: &Vec<String>) -> Result<(), Box<dyn Error>> {
    let mut document = TantivyDocument::default();
    document.add_i64(ft.id_field, id);
    for i in 0..fields.len() {
        let field = ft.schema.get_field(&fields[i])?;
        document.add_text(field, &valus[i]);
    }
    ft.writer.add_document(document)?;
    Ok(())
}

pub fn ft_delete_document(ft: &mut FTIndex, id:i64) -> Result<(), Box<dyn Error>> {
    let term = Term::from_field_i64(ft.id_field, id);
    ft.writer.delete_term(term);
    Ok(())
}

pub fn ft_commit(ft: &mut FTIndex) -> Result<(),  Box<dyn Error>> {
    ft.writer.commit()?;
    ft.reader.reload()?;
    Ok(())
}

pub fn ft_query(ft: &mut FTIndex, query: &String, options: &QueryOptions) -> Result<Vec<IdScore>, Box<dyn Error>> {
    let searcher = ft.reader.searcher();
    let query_parser = QueryParser::for_index(&ft.index, ft.fields.clone());
    let query = query_parser.parse_query(query)?;
    let top_docs = searcher.search(&query, &TopDocs::with_limit(options.top_n))?;
    let mut id_score: Vec<IdScore> =  Vec::new();
    for (_score, doc_address) in top_docs {
        let retrieved_doc: TantivyDocument = searcher.doc(doc_address)?;
        let res = crate::IdScore {
            id: retrieved_doc.get_first(ft.id_field).unwrap().as_i64().unwrap(),
            score: _score
        };
        id_score.push(res);
    }
    Ok(id_score)
}