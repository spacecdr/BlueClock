const fs = require('fs');
const vm = require('vm');
const page = fs.readFileSync('src/page.h', 'utf8');
const match = page.match(/<script>([\s\S]*?)<\/script>/);
if (!match) throw new Error('Panel script missing');
new vm.Script(match[1], { filename: 'panel.js' });
console.log('PASS: panel JavaScript syntax');
