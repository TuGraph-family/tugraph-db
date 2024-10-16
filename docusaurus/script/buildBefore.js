/**
 * file: build 前生成文档的映射文件
 * author: Allen
*/

const fs = require('fs');
const path = require('path');

// 定义要遍历的根目录
const rootDir = path.join(__dirname, '../../docs/zh-CN/source');

// 创建一个对象来存储映射
const folderMappings = {};

// 递归读取目录中的文件夹
const readDirectories = (dirPath) => {
  fs.readdir(dirPath, (err, items) => {
    if (err) {
      console.error('无法读取目录', err);
      return;
    }

    // 遍历每个项目
    items.forEach((item) => {
      const itemPath = path.join(dirPath, item);
      
      // 检查是否为文件夹
      fs.stat(itemPath, (err, stats) => {
        if (err) {
          console.error('无法读取文件夹信息', err);
          return;
        }

        if (stats.isDirectory()) {
          // 检查 index.rst 文件是否存在
          const indexPath = path.join(itemPath, 'index.rst');
          fs.access(indexPath, fs.constants.F_OK, (err) => {
            if (!err) {
              // 读取 index.rst 文件的第一行
              fs.readFile(indexPath, 'utf8', (err, data) => {
                if (err) {
                  console.error('无法读取文件', indexPath, err);
                  return;
                }
                
                const firstLine = data.split('\n')[0].trim();
                const cleanKey = item.replace(/^\d+\./, ''); // 去掉前面的数字和点
                folderMappings[cleanKey] = firstLine;
              });
            }

            // 递归读取子文件夹
            readDirectories(itemPath);
          });
        }
      });
    });
  });
};

// 开始读取目录
readDirectories(rootDir);

// 输出文件夹映射到 JSON 文件
setTimeout(() => {
  const outputPath = path.join(__dirname, 'folderMappings.json');
  fs.writeFile(outputPath, JSON.stringify(folderMappings, null, 2), 'utf8', (err) => {
    if (err) {
      console.error('无法写入 JSON 文件', err);
    } else {
      console.log('文件夹名称映射已写入', outputPath);
    }
  });
}, 3000); // 设置延时以确保所有读取操作完成
