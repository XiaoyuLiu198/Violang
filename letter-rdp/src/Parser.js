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

    /**
    * StatementList
    *   :  Statement Statement Statement
    *   ;
    */
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


    /**
    * Block
    *   :  StatementList
    */
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

    /**
    * Expression
    *   :  Literal
        ;
    */
    Expression(){
        return this.AssignmentExpression();
    }

    /**
    * AssignmentExpression
    *   :  AdditiveExpression
    *   |  LeftHandSideExpression AssignmentOperator AssignmentExpression
    *   ;
    */
    AssignmentExpression(){
        const left = this.AdditiveExpression();

        if (!this._isAssignmentOperator(this._lookahead.type)){
            return left;
        }
        return {
            type: 'AssignmentExpression',
            operator: this.AssignmentOperator().value,
            left: this._checkValidAssignmentTarget(left),
            right: this.AssignmentExpression(),
        };
    }

    /**
    * LeftHandSideExpression
    *   :  Identifier
        ;
    */
    LeftHandSideExpression(){
        return this.Identifier();
    }

    Identifier(){
        const name = this._eat('IDENTIFIER').value;
        return {
            type: 'Identifier',
            name,
        };
    }

    _checkValidAssignmentTarget(){
        if (node.type === 'Identifier'){
            return node;
        }
        throw new SyntaxError('Invalid left-hand side')
    }

    AssignmentOperator(){
        if (this._lookahead.type ==== 'SIMPLE_ASSIGN'){
            return this._eat('SIMPLE_ASSIGN');
        }
        return this._eat('COMPLEX_ASSIGN');
    }

    AdditiveExpression(){
        return this._BinaryExpression(
            'MultiplicativeExpression',
            'ADDITIVE_OPERATOR',
        );
    }

    _BinaryExpression(builderName, operatorToken) {
        let left = this[builderName]();

        while (this._lookahead.type === operatorToken) {
            const operator = this._eat(operatorToken).value;

            const right = this[builderName]();

            left = {
                type: 'BinaryExpression',
                operator,
                left,
                right,
            };
        }

        return left;
    }

   /**
    * PrimaryExpression
    *   :  Literal
    *   |  ParenthesizedExpression
    *   |  LeftHandSideExpression
    *   ;
    */
    PrimaryExpression() {
        switch (this._lookahead.type) {
            case '(':
                return this.ParentthesizedExpression();
            default:
                return this.Literal();
        }
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