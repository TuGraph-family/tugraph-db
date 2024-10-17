const fs = require('fs-extra');
const path = require('path');
const { exec } = require('child_process');

const originalDocsDir = '../docs';
const tempDocsDir = './docs';

async function convertRstToMd() {
  try {

    /** 先清除 docusaurus/docs 目录, 复制原始文档目录到临时目录 */
    await fs.remove(tempDocsDir);
    await fs.copy(originalDocsDir, tempDocsDir);
    console.log('文档已复制到临时目录');

    // 递归转换函数
    async function convertDir(dir) {
      const items = await fs.readdir(dir);

      for (const item of items) {
        const fullPath = path.join(dir, item);
        const stat = await fs.stat(fullPath);

        if (stat.isDirectory()) {
          await convertDir(fullPath);
        }

        else if (path.extname(item) === '.rst' && !['index.rst'].includes(item)) {
          const outputFile = path.join(dir, path.basename(item, '.rst') + '.md');
          const command = `pandoc -f rst -t gfm -o "${outputFile}" "${fullPath}"`;
          await new Promise((resolve, reject) => {
            exec(command, (err, stdout, stderr) => {
              if (err) {
                console.error(`转换文件时出错: ${stderr}`);
                reject(err);
              } else {
                console.log(`已成功转换: ${fullPath} 到 ${outputFile}`);
                resolve();
              }
            });
          });
          /** 删除转换后的rst文件 */
          await fs.unlink(fullPath);
        }
      }
    }
    await convertDir(tempDocsDir);

    // 执行Docusaurus构建
    await new Promise((resolve, reject) => {
      const args = process.argv.slice(2)[0];
      const execArgs = 'yarn docusaurus:' + args;
      exec(execArgs, { cwd: tempDocsDir }, (err, stdout, stderr) => {
        if (err) {
          console.error(`构建时出错: ${stderr}`);
          reject(err);
        } else {
          console.log('Docusaurus构建成功');
          resolve();
        }
      });
    });

    // 删除临时目录
    await fs.remove(tempDocsDir);
    console.log('临时目录已删除');
  } catch (err) {
    console.error(`处理过程中出错: ${err}`);
  }
}

process.on('SIGINT', async () => {
  console.log('捕捉到主进程 SIGINT 信号');
  await fs.remove(tempDocsDir);
  process.exit();
});

// 监听主进程的 SIGTERM 信号
process.on('SIGTERM', async () => {
  console.log('捕捉到主进程 SIGTERM 信号');
  await fs.remove(tempDocsDir);
  process.exit();
});

convertRstToMd();