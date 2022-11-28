const {Tokenizer} = require('./Tokenizer');

class Parser{

 /**
 * initialize tokenizer
 */
  constructor(){
    this._string = '';
    this._tokenizer = new Tokenizer();
  }

  /**
   * Parse string to AST
   */
  parse(string) {
    this._string = string;
    this._tokenizer.init(string);

    //predictive parse by looking one token ahead
    this._lookahead = this._tokenizer.getNextToken();
    return this.Program();
  }

  /**
   * Main entry
   * Program
   *    : Literal
   */
   Program() {
    return {
        type: 'Program',
        body: this.StatementList(),
    };
    }

    StatementList(stopLookahead = null) {
        const statementList = [this.Statement()];

        while (this._lookahead != null && this._lookahead.type !== stopLookahead) {
            statementList.push(this.Statement());
        };

        return statementList;
    }

    Statement(){
    switch (this._lookahead.type) {
        case '{':
            return this.BlockStatement();
        default:
            return this.ExpressionStatement();
    }
        return this.ExpressionStatement();
    }

    BlockStatement(){
        this._eat('{');

        const body = this._lookahead.type !== '}' ? this.StatementList() : [];

        this._eat('}');
        return {
            type: 'BlockStatement',
            body,
        }
    }

    ExpressionStatement(){
        const expression = this.Expression();
        this._eat(';');
        return {
            type: 'ExpressionStatement',
            expression,
        };
    }

    Expression(){
        return this.literal();
    }

   /**
    * Literal
    *   :  NumericLiteral
    *   |  StringLiteral
    *   ;
    */
    Literal(){
        switch (this._lookahead.type) {
          case  'NUMBER': return this.NumericLiteral();
          case 'STRING': return this.StringLiteral();
        }
    }

   /**
    * StringLiteral
    *   : String
    */
    StringLiteral(){
        const token = this._eat('STRING');
        return {
            type: 'STRING',
            body: token.value.slice(1, -1),
        };
    }

   /**
    * Numerical
    *   : Number
    */
    NumericLiteral() {
    const token = this._eat('NUMBER');
    return {
        type: 'NumericLiteral',
        value: Number(token.value),
    };
    }

    _eat(tokenType) {
    const token = this._lookahead;

    if (token==null){
        throw new SyntaxError(
            'Unexpected end of input, expected: "${tokenType}"',
        );
    }

    if (tokenType !== token.type){
        throw new SyntaxError(
            'Unexpected token type',
        );
    }

    this._lookahead = this._tokenizer.getNextToken();
    return token;
    }
}

module.exports = {
    Parser,
};