// Durham Language Interpreter for Web
class DurhamCompiler {
    constructor() {
        this.output = [];
        this.variables = {};
        this.functions = {};
        this.stringVariables = {};
        
        // College name to number mapping
        this.colleges = {
            'butler': 0, 'chads': 1, 'marys': 2, 'collingwood': 3,
            'johns': 4, 'castle': 5, 'cuths': 6, 'trevs': 7,
            'aidans': 8, 'snow': 9, 'grey': 10, 'stephenson': 11,
            'hatfield': 12, 'hildbede': 13, 'south': 14,
            'vanmildert': 15, 'ustinov': 16
        };
    }

    compile(code) {
        this.output = [];
        this.variables = {};
        this.functions = {};
        this.stringVariables = {};
        
        try {
            const lines = this.preprocess(code);
            console.log('Preprocessed statements:', lines);
            this.execute(lines);
            console.log('Output array:', this.output);
            return { success: true, output: this.output.join('\n') };
        } catch (error) {
            console.error('Compilation error:', error);
            return { success: false, output: `Error: ${error.message}` };
        }
    }

    preprocess(code) {
        // First, remove comment lines (lines that are only " comment ")
        // Comments are standalone lines starting and ending with quotes
        const lines = code.split('\n');
        const cleanedLines = lines.filter(line => {
            const trimmed = line.trim();
            // Remove lines that are just comments: " text "
            if (trimmed.match(/^"[^"]*"$/)) {
                return false;
            }
            return true;
        });
        
        const cleanedCode = cleanedLines.join('\n');
        
        // Now split into statements by period
        const statements = cleanedCode.split('.').map(s => s.trim()).filter(s => s.length > 0);
        
        return statements;
    }

    execute(statements) {
        let i = 0;
        while (i < statements.length) {
            const stmt = statements[i].trim();
            if (!stmt) {
                i++;
                continue;
            }

            console.log(`Executing statement ${i}:`, stmt);

            // Print statement
            if (stmt.startsWith('tlc begin')) {
                this.executePrint(stmt);
            }
            // Number variable declaration
            else if (stmt.startsWith('number ')) {
                this.executeNumberDeclaration(stmt);
            }
            // Text variable declaration
            else if (stmt.startsWith('text ')) {
                this.executeTextDeclaration(stmt);
            }
            // Function declaration
            else if (stmt.startsWith('function ')) {
                const funcResult = this.parseFunctionDeclaration(statements, i);
                this.functions[funcResult.name] = funcResult;
                i = funcResult.endIndex;
            }
            // If statement
            else if (stmt.startsWith('if begin')) {
                const ifResult = this.executeIfStatement(statements, i);
                i = ifResult.endIndex;
            }
            // For loop
            else if (stmt.startsWith('for begin')) {
                const forResult = this.executeForLoop(statements, i);
                i = forResult.endIndex;
            }
            // Variable assignment (simple)
            else if (stmt.includes(' is ')) {
                this.executeAssignment(stmt);
            }

            i++;
        }
    }

    executePrint(stmt) {
        // Extract content between "begin" and "end" - use non-greedy match
        const match = stmt.match(/tlc\s+begin\s+(.+?)\s+end/);
        if (!match) {
            console.log('No match for print statement:', stmt);
            return;
        }

        const content = match[1].trim();
        console.log('Print content:', content);

        // Check if it's a string literal
        if (content.startsWith('"') && content.endsWith('"')) {
            const output = content.slice(1, -1);
            console.log('Printing string literal:', output);
            this.output.push(output);
        }
        // Check if it's an expression with durham (concatenation)
        else if (content.includes(' durham ')) {
            const result = this.evaluateExpression(content);
            console.log('Printing concatenation:', result);
            this.output.push(result);
        }
        // Check if it's a variable
        else if (this.stringVariables[content]) {
            console.log('Printing string variable:', this.stringVariables[content]);
            this.output.push(this.stringVariables[content]);
        }
        else if (this.variables[content] !== undefined) {
            console.log('Printing numeric variable:', this.variables[content]);
            this.output.push(String(this.variables[content]));
        }
        // Evaluate as expression
        else {
            const value = this.evaluateExpression(content);
            console.log('Printing expression result:', value);
            this.output.push(String(value));
        }
    }

    executeNumberDeclaration(stmt) {
        // number x is castle.
        const match = stmt.match(/number\s+(\w+)\s+is\s+(.+)/);
        if (match) {
            const varName = match[1];
            const expression = match[2].trim();
            this.variables[varName] = this.evaluateExpression(expression);
        }
    }

    executeTextDeclaration(stmt) {
        // text name is begin "value" end.
        const match = stmt.match(/text\s+(\w+)\s+is\s+begin\s+"([^"]+)"\s+end/);
        if (match) {
            const varName = match[1];
            const value = match[2];
            this.stringVariables[varName] = value;
            return;
        }

        // text result is expr durham expr
        const match2 = stmt.match(/text\s+(\w+)\s+is\s+(.+)/);
        if (match2) {
            const varName = match2[1];
            const expression = match2[2].trim();
            this.stringVariables[varName] = this.evaluateExpression(expression);
        }
    }

    executeAssignment(stmt) {
        const parts = stmt.split(' is ');
        if (parts.length !== 2) return;

        const varName = parts[0].trim();
        const expression = parts[1].trim();
        this.variables[varName] = this.evaluateExpression(expression);
    }

    executeIfStatement(statements, startIndex) {
        const stmt = statements[startIndex];
        
        // Extract condition
        const condMatch = stmt.match(/if begin (.+?) end front/);
        if (!condMatch) return { endIndex: startIndex };

        const condition = condMatch[1];
        const conditionResult = this.evaluateCondition(condition);

        // Find the matching "back" and possible "else"
        let depth = 1;
        let thenStatements = [];
        let elseStatements = [];
        let inElse = false;
        let i = startIndex + 1;

        while (i < statements.length && depth > 0) {
            const s = statements[i].trim();
            
            if (s.startsWith('if begin') || s.startsWith('for begin') || s.startsWith('while begin')) {
                depth++;
            }
            
            if (s === 'back') {
                depth--;
                if (depth === 0) break;
            }
            
            if (s === 'back else front' && depth === 1) {
                inElse = true;
                i++;
                continue;
            }

            if (depth > 0) {
                if (inElse) {
                    elseStatements.push(s);
                } else {
                    thenStatements.push(s);
                }
            }
            
            i++;
        }

        // Execute appropriate branch
        if (conditionResult) {
            this.execute(thenStatements);
        } else if (elseStatements.length > 0) {
            this.execute(elseStatements);
        }

        return { endIndex: i };
    }

    executeForLoop(statements, startIndex) {
        // For loop is split across multiple statements:
        // statements[i]:     "for begin i is butler"
        // statements[i+1]:   "i lesser snow"
        // statements[i+2]:   "i is i durham chads end front"
        
        let forHeader = statements[startIndex];
        let headerEndIndex = startIndex;
        
        // Collect all parts until we find "end front"
        while (headerEndIndex < statements.length && !forHeader.includes('end front')) {
            headerEndIndex++;
            if (headerEndIndex < statements.length) {
                forHeader += '. ' + statements[headerEndIndex];
            }
        }
        
        console.log('Raw collected header:', forHeader);
        console.log('Header ended at index:', headerEndIndex);
        
        // Check if the statement containing "end front" has more content after it
        // If so, we need to split it and keep the remainder for the body
        let remainderAfterHeader = '';
        const endFrontIndex = forHeader.indexOf('end front');
        if (endFrontIndex !== -1) {
            const afterEndFront = forHeader.substring(endFrontIndex + 9).trim(); // +9 for "end front"
            if (afterEndFront.length > 0) {
                remainderAfterHeader = afterEndFront;
                console.log('Found remainder after end front:', remainderAfterHeader);
            }
            forHeader = forHeader.substring(0, endFrontIndex + 9); // +9 for "end front"
        }
        
        console.log('Cleaned for loop header:', forHeader);
        
        // Now parse: for begin init. condition. increment end front
        const match = forHeader.match(/for begin\s+(.+?)\s+end\s+front/);
        if (!match) {
            console.log('No match for for loop:', forHeader);
            return { endIndex: startIndex };
        }

        // Split the content by periods
        const forContent = match[1].trim();
        const parts = forContent.split(/\.\s*/);
        
        if (parts.length < 3) {
            console.log('For loop needs 3 parts, got:', parts);
            return { endIndex: headerEndIndex };
        }

        const init = parts[0].trim();
        const condition = parts[1].trim();
        const increment = parts[2].trim();
        
        console.log('For loop parts:', { init, condition, increment });

        // Find loop body (start from after the header)
        let depth = 1;
        let bodyStatements = [];
        
        // If there was a remainder after "end front", add it as first body statement
        if (remainderAfterHeader) {
            bodyStatements.push(remainderAfterHeader);
        }
        
        let bodyIndex = headerEndIndex + 1;

        console.log('Looking for body starting at index', bodyIndex, 'statement:', statements[bodyIndex]);

        while (bodyIndex < statements.length && depth > 0) {
            const s = statements[bodyIndex].trim();
            
            console.log(`Checking statement[${bodyIndex}]:`, s, 'depth:', depth);
            
            if (s.startsWith('for begin') || s.startsWith('if begin') || s.startsWith('while begin')) {
                depth++;
            }
            
            if (s === 'back') {
                depth--;
                console.log('Found back, depth now:', depth);
                if (depth === 0) break;
            }

            if (depth > 0) {
                console.log('Adding to body:', s);
                bodyStatements.push(s);
            }
            
            bodyIndex++;
        }

        console.log('For loop body:', bodyStatements);

        // Execute loop
        this.executeAssignment(init);
        
        let iterations = 0;
        const maxIterations = 10000;
        
        while (this.evaluateCondition(condition) && iterations < maxIterations) {
            this.execute(bodyStatements);
            this.executeAssignment(increment);
            iterations++;
        }

        if (iterations >= maxIterations) {
            throw new Error('Loop exceeded maximum iterations');
        }

        return { endIndex: bodyIndex };
    }

    parseFunctionDeclaration(statements, startIndex) {
        // function name begin params end front
        const stmt = statements[startIndex];
        const match = stmt.match(/function\s+(\w+)\s+begin\s+(.+?)\s+end\s+front/);
        
        if (!match) return { endIndex: startIndex };

        const funcName = match[1];
        const params = match[2].split(' and ').map(p => p.trim());

        // Find function body
        let depth = 1;
        let bodyStatements = [];
        let i = startIndex + 1;

        while (i < statements.length && depth > 0) {
            const s = statements[i].trim();
            
            if (s.startsWith('function ') || s.startsWith('for begin') || s.startsWith('if begin')) {
                depth++;
            }
            
            if (s === 'back') {
                depth--;
                if (depth === 0) break;
            }

            if (depth > 0) {
                bodyStatements.push(s);
            }
            
            i++;
        }

        return {
            name: funcName,
            params: params,
            body: bodyStatements,
            endIndex: i
        };
    }

    evaluateCondition(condition) {
        // Handle comparisons: x lesser y, x greater y, x equals y
        if (condition.includes(' lesser ')) {
            const parts = condition.split(' lesser ');
            const left = this.evaluateExpression(parts[0].trim());
            const right = this.evaluateExpression(parts[1].trim());
            return left < right;
        }
        if (condition.includes(' greater ')) {
            const parts = condition.split(' greater ');
            const left = this.evaluateExpression(parts[0].trim());
            const right = this.evaluateExpression(parts[1].trim());
            return left > right;
        }
        if (condition.includes(' equals ')) {
            const parts = condition.split(' equals ');
            const left = this.evaluateExpression(parts[0].trim());
            const right = this.evaluateExpression(parts[1].trim());
            return left === right;
        }
        return false;
    }

    evaluateExpression(expr) {
        expr = expr.trim();

        // String literal
        if (expr.startsWith('"') && expr.endsWith('"')) {
            return expr.slice(1, -1);
        }

        // String in begin...end
        const stringMatch = expr.match(/begin\s+"([^"]+)"\s+end/);
        if (stringMatch) {
            return stringMatch[1];
        }

        // String concatenation
        if (expr.includes(' durham ')) {
            const parts = expr.split(' durham ').map(p => p.trim());
            let result = '';
            for (const part of parts) {
                const val = this.evaluateExpression(part);
                result += String(val);
            }
            return result;
        }

        // Arithmetic operations
        if (expr.includes(' york ')) { // multiplication
            const parts = expr.split(' york ');
            return this.evaluateExpression(parts[0]) * this.evaluateExpression(parts[1]);
        }
        if (expr.includes(' edinburgh ')) { // division
            const parts = expr.split(' edinburgh ');
            return Math.floor(this.evaluateExpression(parts[0]) / this.evaluateExpression(parts[1]));
        }
        if (expr.includes(' newcastle ')) { // subtraction
            const parts = expr.split(' newcastle ');
            return this.evaluateExpression(parts[0]) - this.evaluateExpression(parts[1]);
        }

        // Function call
        if (expr.includes(' begin ') && expr.includes(' and ')) {
            const funcMatch = expr.match(/(\w+)\s+begin\s+(.+?)\s+end/);
            if (funcMatch && this.functions[funcMatch[1]]) {
                const funcName = funcMatch[1];
                const args = funcMatch[2].split(' and ').map(a => this.evaluateExpression(a.trim()));
                return this.callFunction(funcName, args);
            }
        }

        // Variable reference
        if (this.stringVariables[expr]) {
            return this.stringVariables[expr];
        }
        if (this.variables[expr] !== undefined) {
            return this.variables[expr];
        }

        // College name (with comma for multi-digit)
        if (expr.includes(',')) {
            const parts = expr.split(',').map(p => p.trim());
            let result = '';
            for (const part of parts) {
                if (this.colleges[part] !== undefined) {
                    result += String(this.colleges[part]);
                }
            }
            return parseInt(result);
        }

        // Single college name
        if (this.colleges[expr] !== undefined) {
            return this.colleges[expr];
        }

        // Number literal
        const num = parseInt(expr);
        if (!isNaN(num)) {
            return num;
        }

        return expr;
    }

    callFunction(name, args) {
        const func = this.functions[name];
        if (!func) return 0;

        // Save current variables
        const savedVars = { ...this.variables };

        // Set parameters
        func.params.forEach((param, i) => {
            this.variables[param] = args[i] || 0;
        });

        // Execute function body
        let returnValue = 0;
        for (const stmt of func.body) {
            if (stmt.startsWith('mcs begin')) {
                const match = stmt.match(/mcs begin (.+?) end/);
                if (match) {
                    returnValue = this.evaluateExpression(match[1]);
                    break;
                }
            } else {
                this.execute([stmt]);
            }
        }

        // Restore variables
        this.variables = savedVars;

        return returnValue;
    }
}

// Export for use in app.js
window.DurhamCompiler = DurhamCompiler;
