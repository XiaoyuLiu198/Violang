/**
 * test
 */
 const {Parser} = require('../src/Parser');
 const parser = new Parser();
 const program = '88';
 const ast = parser.parse(program);

  const program = "hello";
 const ast = parser.parse(program);

 console.log(JSON.stringify(ast, null, 2))