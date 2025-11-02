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
    'for', 'if', 'while', 'function',
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
// ========== TYPING GAME SECTION (COMMENTED OUT) ==========
/*
// Typing game state
let gameActive = false;
let gameTarget = '';
let gameStartTime = 0;
let gameStartPosition: vscode.Position | null = null;
let gameStatusBar: vscode.StatusBarItem;

// Decoration types for typing game
let targetDecorationType: vscode.TextEditorDecorationType;
let errorDecorationType: vscode.TextEditorDecorationType;

// WPM tracking
let typingHistory: { time: number; chars: number }[] = [];
let lastTypingTime = 0;
const WPM_THRESHOLD = 100;
const WPM_WINDOW_MS = 3000; // Track WPM over 3 second window

// Typing game challenges
const GAME_CHALLENGES = [
    'function sumfunc begin x and y end front mcs x durham y. back.',
    'for i in butler to josephine front tlc begin i end. back.',
    'x is chads durham marys.',
    'if begin x greater butler end front tlc begin "success" end. back.',
    'result is sumfunc begin chads and marys end.',
    'array is new college begin ustinov end.',
    'while x edinburgh josephine front x is x durham hatfield. back.',
    '" This is a comment "',
];

// Function to calculate current WPM
function calculateCurrentWPM(): number {
    const now = Date.now();
    // Remove old entries outside the window
    typingHistory = typingHistory.filter(entry => now - entry.time < WPM_WINDOW_MS);
    
    if (typingHistory.length === 0) return 0;
    
    const totalChars = typingHistory.reduce((sum, entry) => sum + entry.chars, 0);
    const timeSpanSeconds = (now - typingHistory[0].time) / 1000;
    
    if (timeSpanSeconds < 1) return 0;
    
    // WPM = (chars / 5) / (time in minutes)
    const wpm = Math.round((totalChars / 5) / (timeSpanSeconds / 60));
    return wpm;
}

// Update decorations to show target text and errors
function updateGameDecorations(editor: vscode.TextEditor) {
    if (!gameActive || !gameStartPosition || !gameTarget) return;
    
    const endPosition = editor.document.lineAt(editor.document.lineCount - 1).range.end;
    const typedText = editor.document.getText(new vscode.Range(gameStartPosition, endPosition));
    
    const decorations: vscode.DecorationOptions[] = [];
    const errorDecorations: vscode.DecorationOptions[] = [];
    
    // Show remaining text to type (in gray) - always show full target at the beginning
    if (typedText.length === 0) {
        // Show full target when nothing typed yet
        decorations.push({
            range: new vscode.Range(gameStartPosition, gameStartPosition),
            renderOptions: {
                after: {
                    contentText: gameTarget,
                    color: 'rgba(150, 150, 150, 0.8)',
                    fontStyle: 'italic',
                    margin: '0 0 0 4px'
                }
            }
        });
    } else if (typedText.length < gameTarget.length) {
        // Show remaining text
        const remainingText = gameTarget.substring(typedText.length);
        const hintPosition = new vscode.Position(
            gameStartPosition.line,
            gameStartPosition.character + typedText.length
        );
        
        decorations.push({
            range: new vscode.Range(hintPosition, hintPosition),
            renderOptions: {
                after: {
                    contentText: remainingText,
                    color: 'rgba(150, 150, 150, 0.8)',
                    fontStyle: 'italic'
                }
            }
        });
    }
    
    // Highlight errors in red
    for (let i = 0; i < typedText.length && i < gameTarget.length; i++) {
        if (typedText[i] !== gameTarget[i]) {
            const errorPos = new vscode.Position(
                gameStartPosition.line,
                gameStartPosition.character + i
            );
            errorDecorations.push({
                range: new vscode.Range(errorPos, new vscode.Position(errorPos.line, errorPos.character + 1)),
                hoverMessage: `Expected: '${gameTarget[i]}'`
            });
        }
    }
    
    editor.setDecorations(targetDecorationType, decorations);
    editor.setDecorations(errorDecorationType, errorDecorations);
}

// Function to start the typing game
async function startTypingGame(editor: vscode.TextEditor, context: vscode.ExtensionContext, autoTriggered: boolean = false) {
    gameTarget = GAME_CHALLENGES[Math.floor(Math.random() * GAME_CHALLENGES.length)];
    gameActive = true;
    gameStartTime = Date.now();
    typingHistory = []; // Reset WPM tracking

    // Disable regular autocomplete during game
    if (completionProvider) {
        completionProvider.dispose();
        completionProvider = null;
    }

    // Move to end of document and insert new line
    const lastLine = editor.document.lineCount - 1;
    const lastLineText = editor.document.lineAt(lastLine).text;
    const endPosition = new vscode.Position(lastLine, lastLineText.length);
    
    await editor.edit(editBuilder => {
        // Add newline if not at start of empty line
        if (lastLineText.trim().length > 0) {
            editBuilder.insert(endPosition, '\n');
        }
    });

    // Store start position for later deletion
    gameStartPosition = new vscode.Position(editor.document.lineCount - 1, 0);
    
    // Move cursor to the new line
    editor.selection = new vscode.Selection(gameStartPosition, gameStartPosition);
    
    // Show initial hint
    updateGameDecorations(editor);

    gameStatusBar.text = `$(watch) Type: ${gameTarget}`;
    gameStatusBar.tooltip = 'Type the text shown! Speed matters!';
    
    const message = autoTriggered
        ? `🚀 Speed Demon Detected! Complete this challenge: "${gameTarget}"`
        : `🎮 Type: "${gameTarget}" as fast as you can!`;
    
    vscode.window.showInformationMessage(message);
    // Text-to-speech for speed demon
    if (autoTriggered) {
        speak('Speed demon detected!');
    }
}

// Function to cancel typing game
function cancelTypingGame(editor: vscode.TextEditor, context: vscode.ExtensionContext) {
    if (!gameActive) return;
    
    gameActive = false;
    
    // Clear decorations
    editor.setDecorations(targetDecorationType, []);
    editor.setDecorations(errorDecorationType, []);
    
    // Delete the game text if it exists
    if (gameStartPosition) {
        editor.edit(editBuilder => {
            const endPosition = editor.document.lineAt(editor.document.lineCount - 1).range.end;
            const deleteRange = new vscode.Range(gameStartPosition!, endPosition);
            editBuilder.delete(deleteRange);
        });
    }
    
    // Re-enable regular autocomplete
    completionProvider = vscode.languages.registerCompletionItemProvider(
        { language: 'durham', scheme: 'file' },
        new DurhamCompletionProvider(),
        ...DURHAM_KEYWORDS[0].split('')
    );
    context.subscriptions.push(completionProvider);
    
    gameStatusBar.text = '$(game) Durham Typing Game';
    gameStatusBar.tooltip = 'Click to start typing challenge!';
    
    vscode.window.showInformationMessage('🚫 Typing challenge cancelled');
}
*/
// ========== END TYPING GAME SECTION ==========
// Completion provider initialization
let completionProvider = null;
function activate(context) {
    console.log('Durham language extension is now active!');
    // TYPING GAME INITIALIZATION COMMENTED OUT
    /*
    // Create decoration types for typing game
    targetDecorationType = vscode.window.createTextEditorDecorationType({});
    errorDecorationType = vscode.window.createTextEditorDecorationType({
        backgroundColor: 'rgba(255, 0, 0, 0.3)',
        border: '1px solid red'
    });
    */
    // Register completion provider
    completionProvider = vscode.languages.registerCompletionItemProvider({ language: 'durham', scheme: 'file' }, new DurhamCompletionProvider(), ...DURHAM_KEYWORDS[0].split('') // Trigger on any character
    );
    // TYPING GAME COMMANDS AND STATUS BAR COMMENTED OUT
    /*
    // Create status bar item for typing game
    gameStatusBar = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Right, 100);
    gameStatusBar.command = 'durham.startTypingGame';
    gameStatusBar.text = '$(game) Durham Typing Game';
    gameStatusBar.tooltip = 'Click to start typing challenge!';
    gameStatusBar.show();

    // Register typing game command
    const startGameCommand = vscode.commands.registerCommand('durham.startTypingGame', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage('Open a .dur file first!');
            return;
        }

        if (editor.document.languageId !== 'durham') {
            vscode.window.showErrorMessage('This only works with Durham (.dur) files!');
            return;
        }

        await startTypingGame(editor, context, false);
    });

    // Register ESC key command to cancel typing game
    const cancelGameCommand = vscode.commands.registerCommand('durham.cancelTypingGame', () => {
        const editor = vscode.window.activeTextEditor;
        if (editor && gameActive) {
            cancelTypingGame(editor, context);
        }
    });

    // Register keybinding for ESC during game
    context.subscriptions.push(
        vscode.commands.registerCommand('type', async (args) => {
            // Check if ESC was pressed during game
            if (args.text === '\u001b' && gameActive) {
                const editor = vscode.window.activeTextEditor;
                if (editor) {
                    cancelTypingGame(editor, context);
                    return;
                }
            }
            // Otherwise, execute default type command
            await vscode.commands.executeCommand('default:type', args);
        })
    );
    */
    const textChangeListener = vscode.workspace.onDidChangeTextDocument(event => {
        if (event.document.languageId === 'durham' && event.contentChanges.length > 0) {
            // TYPING GAME WPM TRACKING COMMENTED OUT
            /*
            const change = event.contentChanges[0];
            const text = change.text;

            // Track typing for WPM calculation (only when not in game)
            if (!gameActive && text.length === 1) {
                const now = Date.now();
                typingHistory.push({ time: now, chars: 1 });
                
                // Check if WPM exceeds threshold
                const currentWPM = calculateCurrentWPM();
                if (currentWPM > WPM_THRESHOLD) {
                    const editor = vscode.window.activeTextEditor;
                    if (editor && editor.document.languageId === 'durham') {
                        // Trigger typing game automatically!
                        startTypingGame(editor, context, true);
                        return;
                    }
                }
            }

            // Check typing game progress
            if (gameActive && gameStartPosition) {
                const editor = vscode.window.activeTextEditor;
                if (!editor) return;
                
                // Get text from game start position to current position (don't trim - exact match)
                const currentLine = editor.document.lineAt(gameStartPosition.line);
                const gameText = editor.document.getText(new vscode.Range(
                    gameStartPosition,
                    currentLine.range.end
                ));
                
                // Check if completed (exact match)
                if (gameText === gameTarget) {
                    const timeTaken = ((Date.now() - gameStartTime) / 1000).toFixed(2);
                    const wpm = Math.round((gameTarget.length / 5) / (parseFloat(timeTaken) / 60));
                    
                    gameActive = false;
                    
                    // Delete the game text immediately
                    editor.edit(editBuilder => {
                        const deleteRange = new vscode.Range(
                            gameStartPosition!,
                            currentLine.range.end
                        );
                        editBuilder.delete(deleteRange);
                    }).then(() => {
                        // Clear decorations after deletion
                        editor.setDecorations(targetDecorationType, []);
                        editor.setDecorations(errorDecorationType, []);
                    });
                    
                    gameStatusBar.text = `$(trophy) WPM: ${wpm} | Time: ${timeTaken}s`;
                    gameStatusBar.tooltip = 'Click to play again!';
                    
                    // Re-enable regular autocomplete after game
                    completionProvider = vscode.languages.registerCompletionItemProvider(
                        { language: 'durham', scheme: 'file' },
                        new DurhamCompletionProvider(),
                        ...DURHAM_KEYWORDS[0].split('')
                    );
                    context.subscriptions.push(completionProvider);
                    
                    vscode.window.showInformationMessage(`🏆 Complete! Time: ${timeTaken}s | WPM: ${wpm}`);
                    
                    return;
                }
                
                // Update decorations to show progress and errors on every keystroke
                updateGameDecorations(editor);
                
                // Update status bar with progress
                const progress = Math.min(100, Math.round((gameText.length / gameTarget.length) * 100));
                gameStatusBar.text = `$(watch) ${progress}% | Type: ${gameTarget}`;
            }
            */
            // TTS disabled during normal coding - only used for "Speed Demon Detected"
            // Removed word voicing to avoid interruption during coding
        }
    });
    if (completionProvider) {
        context.subscriptions.push(completionProvider);
    }
    context.subscriptions.push(textChangeListener);
}
function deactivate() { }
//# sourceMappingURL=extension.js.map