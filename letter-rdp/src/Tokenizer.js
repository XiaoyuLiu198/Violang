const Spec = [
    [/^\s+/, null],
    [/^\/\*[\s\S]*?\*\/, null],
    [/^:/, ';'],
    [/^\{/, '{'],
    [/^\}/, '}'],
    [/^\d+/, 'NUMBER'],
    [/^"[^"]*"/, 'STRING''],
    [/^''[^']*'/, 'STRING''],
]

class Tokenizer{

    isEOF() {
        return this._cursor === this._string.length
    }

    init(string){
        this._string = string;
        this._cursor = 0;
    }

    hasMoreToken(){
        return this._cursor < this._string.length;
    }

    getNextToken(){
        if (!this.hasMoreTokens()){
           return null;
        }

        const string = this._string.slice(this._cursor);

        for (const [regexp, tokenType] of Spec) {
            const tokenValue = this._match(regexp, string);

            if (tokenValue == null){
                continue;
            }
            return {
                type: tokenType,
                value: tokenValue,
            };
            throw new SyntaxError('Unexpected token: "${string[0]}"');
        }

        // Number token;
        _match(regexp, string) {
            const matched = regexp.exec(string);
            if(matched == null){
                return null;
            }
            else {
                this._cursor += matched[0].length;
                return matched[0];
            }
        }


        let matched = /"[^"]*"/.exec(string);
        if (matched !== null) {
            this._cursor += matched[0].length;
            return {
                type: 'STRING',
                value: matched[0],
            };
        }

//        if (!Number.isNaN(Number(string[0]))) {
//            let number = '';
//            while (!Number.isNaN(Number(string[this._cursor]))){
//                number += string[this._cursor++];
//            }
//            return {
//                type: 'NUMBER',
//                value: number,
//            };
//        }

        // String token
//        if (string[0] === '"'){
//            let s = '';
//            do {
//                s += string[this._cursor++];
//            } while(string[this._cursor] !== '"' && !this.isEOF());
//            this._cursor++;
//            return {
//                type: 'STRING';
//                value: s,
//              };
//            }
            return null;
        }
}

module.exports = {
    Tokenizer;
}