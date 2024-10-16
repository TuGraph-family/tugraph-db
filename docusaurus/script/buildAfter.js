/**
 * file: build 后根据 foldMapping 重命名中文文档
 * author: Allen
*/

const fs = require('fs');
const path = require('path');

// 读取文件夹映射的 JSON 文件
const folderMappingsPath = path.resolve(__dirname, 'folderMappings.json');
let folderMappings = {};

if (fs.existsSync(folderMappingsPath)) {
  const data = fs.readFileSync(folderMappingsPath, 'utf-8');
  folderMappings = JSON.parse(data);
}

// 递归函数用于遍历目录
function renameDirectories(dirPath) {
  if (!fs.existsSync(dirPath)) {
    console.error(`Directory does not exist: ${dirPath}`);
    return;
  }

  const items = fs.readdirSync(dirPath, { withFileTypes: true });

  for (const item of items) {
    const itemPath = path.join(dirPath, item.name);

    if (item.isDirectory()) {
      console.log(`Checking directory: ${item.name}`);
      
      // 检查是否在 folderMappings 中
      if (folderMappings.hasOwnProperty(item.name)) {
        const newDirName = path.join(dirPath, folderMappings[item.name]);
        
        console.log(`Attempting to rename ${itemPath} to ${newDirName}`);

        // 重命名目录
        if (itemPath !== newDirName) {
          try {
            fs.renameSync(itemPath, newDirName);
            console.log(`Directory renamed from ${itemPath} to ${newDirName}`);
            // 更新 itemPath 以继续递归检查子目录
            renameDirectories(newDirName);
          } catch (err) {
            console.error(`Failed to rename ${itemPath} to ${newDirName}:`, err);
          }
        } else {
          // 如果未重命名，继续递归检查子目录
          renameDirectories(itemPath);
        }
      } else {
        // 递归检查子目录
        renameDirectories(itemPath);
      }
    }
  }
}

// 开始遍历 build/zh 目录
const rootDir = path.resolve(__dirname, '../build/zh');
renameDirectories(rootDir);
