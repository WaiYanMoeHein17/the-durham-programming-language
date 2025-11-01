"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = require("vscode");
const child_process_1 = require("child_process");
// Durham language keywords - matches TokenType enum from tokenizer.h
const DURHAM_KEYWORDS = [
    // Numeric colleges (0-16)
    'butler', 'chads', 'marys', 'collingwood', 'johns', 'castle', 'cuths',
    'trevs', 'aidans', 'snow', 'grey', 'stephenson', 'hatfield', 'hildbede',
    'south', 'vanmildert', 'ustinov',
    // Arithmetic operators
    'durham', 'newcastle', 'york', 'edinburgh',
    // I/O
    'tlc', 'mcs',
    // Control flow
    'for', 'if', 'while',
    // Vector/Array
    'new', 'college', 'at',
    // Logical operators
    'and', 'or', 'not',
    // Punctuation/keywords
    'begin', 'end', 'front', 'back', 'is'
];
// Levenshtein distance function (same as in your C++ code)
function levenshteinDistance(s1, s2) {
    const len1 = s1.length;
    const len2 = s2.length;
    const dp = Array(len1 + 1).fill(null).map(() => Array(len2 + 1).fill(0));
    for (let i = 0; i <= len1; i++)
        dp[i][0] = i;
    for (let j = 0; j <= len2; j++)
        dp[0][j] = j;
    for (let i = 1; i <= len1; i++) {
        for (let j = 1; j <= len2; j++) {
            if (s1[i - 1] === s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            }
            else {
                dp[i][j] = 1 + Math.min(dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]);
            }
        }
    }
    return dp[len1][len2];
}
// Text-to-speech function
function speak(text) {
    // Use child_process exec to run PowerShell TTS directly
    const escapedText = text.replace(/'/g, "''");
    const command = `powershell -Command "Add-Type -AssemblyName System.Speech; $synth = New-Object System.Speech.Synthesis.SpeechSynthesizer; $synth.Speak('${escapedText}')"`;
    (0, child_process_1.exec)(command, (error) => {
        if (error) {
            console.error('TTS Error:', error);
        }
    });
}
// Completion provider (no voice - voice is handled by text change listener)
class DurhamCompletionProvider {
    provideCompletionItems(document, position) {
        const linePrefix = document.lineAt(position).text.substr(0, position.character);
        const wordMatch = linePrefix.match(/\b(\w+)$/);
        const currentWord = wordMatch ? wordMatch[1] : '';
        // Find suggestions based on Levenshtein distance
        const suggestions = DURHAM_KEYWORDS
            .map(keyword => ({
            keyword,
            distance: levenshteinDistance(currentWord.toLowerCase(), keyword)
        }))
            .filter(({ distance, keyword }) => {
            // Same logic as C++ autocorrect
            if (currentWord.length === 1)
                return false;
            if (currentWord.length === 2)
                return distance === 1 && keyword.length === 3;
            return distance <= 2;
        })
            .sort((a, b) => a.distance - b.distance)
            .slice(0, 5); // Top 5 suggestions
        // Create completion items (no voice here)
        return suggestions.map(({ keyword, distance }) => {
            const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
            item.detail = `Durham keyword (distance: ${distance})`;
            item.insertText = keyword;
            return item;
        });
    }
}
function activate(context) {
    console.log('Durham language extension is now active!');
    // Register completion provider
    const provider = vscode.languages.registerCompletionItemProvider({ language: 'durham', scheme: 'file' }, new DurhamCompletionProvider(), ...DURHAM_KEYWORDS[0].split('') // Trigger on any character
    );
    // Word voicing only on space/delimiter
    let currentWord = '';
    const textChangeListener = vscode.workspace.onDidChangeTextDocument(event => {
        if (event.document.languageId === 'durham' && event.contentChanges.length > 0) {
            const change = event.contentChanges[0];
            const text = change.text;
            if (text.length === 1) {
                // Single character typed
                if (text === ' ' || text === '\n' || text === '\t' || text === '.') {
                    // Space or delimiter - speak the whole word immediately
                    if (currentWord.length > 0) {
                        speak(currentWord);
                        currentWord = '';
                    }
                }
                else if (/[a-zA-Z]/.test(text)) {
                    // Letter typed - add to current word (no speaking yet)
                    currentWord += text;
                }
            }
            else if (change.rangeLength > 0 && text.length === 0) {
                // Deletion - update current word
                currentWord = currentWord.slice(0, -1);
            }
        }
    });
    context.subscriptions.push(provider, textChangeListener);
}
function deactivate() { }
//# sourceMappingURL=extension.js.map