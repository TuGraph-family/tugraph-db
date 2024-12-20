const fs = require('fs-extra');
const path = require('path');

const fetchDocHtmlMap = {
  'version-4.5.0': {
    'zh-CN': {
      '3.C++-procedure': 'https://tugraph-db.readthedocs.io/zh-cn/v4.5.0/9.olap%26procedure/1.procedure/3.C%2B%2B-procedure.html',
      '4.Python-procedure': 'https://tugraph-db.readthedocs.io/zh-cn/v4.5.0/9.olap%26procedure/1.procedure/4.Python-procedure.html'
    },
    'en-US': {
      '3.C++-procedure': 'https://tugraph-db.readthedocs.io/en/v4.5.0/9.olap%26procedure/1.procedure/3.C%2B%2B-procedure.html',
      '4.Python-procedure': 'https://tugraph-db.readthedocs.io/en/v4.5.0/9.olap%26procedure/1.procedure/4.Python-procedure.html'
    }
  },
  'version-4.3.2': {
    'zh-CN': {
      '3.C++-procedure': 'https://tugraph-db.readthedocs.io/zh-cn/v4.3.2/9.olap%26procedure/1.procedure/3.C%2B%2B-procedure.html',
      '4.Python-procedure': 'https://tugraph-db.readthedocs.io/zh-cn/v4.3.2/9.olap%26procedure/1.procedure/4.Python-procedure.html'
    },
    'en-US': {
      '3.C++-procedure': 'https://tugraph-db.readthedocs.io/en/v4.3.2/9.olap%26procedure/1.procedure/3.C%2B%2B-procedure.html',
      '4.Python-procedure': 'https://tugraph-db.readthedocs.io/en/v4.3.2/9.olap%26procedure/1.procedure/4.Python-procedure.html'
    }
  },
  'version-4.3.0': {
    'zh-CN': {
      '3.C++-procedure': 'https://tugraph-db.readthedocs.io/zh-cn/v4.3.0/9.olap%26procedure/1.procedure/3.C%2B%2B-procedure.html',
      '4.Python-procedure': 'https://tugraph-db.readthedocs.io/zh-cn/v4.3.0/9.olap%26procedure/1.procedure/4.Python-procedure.html'
    },
    'en-US': {
      '3.C++-procedure': 'https://tugraph-db.readthedocs.io/en/v4.3.0/9.olap%26procedure/1.procedure/3.C%2B%2B-procedure.html',
      '4.Python-procedure': 'https://tugraph-db.readthedocs.io/en/v4.3.0/9.olap%26procedure/1.procedure/4.Python-procedure.html'
    }
  },
  'version-4.2.0': {
    'zh-CN': {
      '3.C++-procedure': 'https://tugraph-db.readthedocs.io/zh-cn/v4.2.0/9.olap%26procedure/1.procedure/3.C%2B%2B-procedure.html',
      '4.Python-procedure': 'https://tugraph-db.readthedocs.io/zh-cn/v4.2.0/9.olap%26procedure/1.procedure/4.Python-procedure.html'
    },
    'en-US': {
      '3.C++-procedure': 'https://tugraph-db.readthedocs.io/en/v4.2.0/9.olap%26procedure/1.procedure/3.C%2B%2B-procedure.html',
      '4.Python-procedure': 'https://tugraph-db.readthedocs.io/en/v4.2.0/9.olap%26procedure/1.procedure/4.Python-procedure.html'
    }
  }
};

async function fetchAndSaveMarkdown(url, outputFilePath) {
  const fetch = (await import('node-fetch')).default;
  const cheerio = (await import('cheerio'));
  try {
    const response = await fetch(url);
    if (!response.ok) {
      throw new Error(`Network response was not ok ${response.statusText}`);
    }
    const htmlData = await response.text();

    const $ = cheerio.load(htmlData);
    const mainContent = $('div[itemprop="articleBody"]');

    if (mainContent.length) {
      mainContent.find('h1').remove();

      const cleanedHtml = mainContent.html();
      const markdownData = `\n${cleanedHtml}\n`;

      await fs.writeFile(outputFilePath, markdownData, 'utf8');
      console.log(`File has been saved: ${outputFilePath}`);
    } else {
      console.error('No <div itemprop="articleBody"> found in the HTML.');
    }
  } catch (error) {
    console.error(`Fetch operation failed: ${error.message}`);
  }
}

async function processVersionDocs(basePath, version, langMap) {
  for (const [lang, docMap] of Object.entries(langMap)) {
    for (const [docType, url] of Object.entries(docMap)) {
      const mdFilePath = path.join(basePath, version, lang, 'source', '9.olap&procedure', '1.procedure', `${docType}.md`);
      await fetchAndSaveMarkdown(url, mdFilePath);
    }
  }
}

async function main() {
  const basePath = path.join(__dirname, '../versions');
  for (const [version, langMap] of Object.entries(fetchDocHtmlMap)) {
    await processVersionDocs(basePath, version, langMap);
  }
}

main();
