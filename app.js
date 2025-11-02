// Main application logic
document.addEventListener('DOMContentLoaded', function() {
    const codeEditor = document.getElementById('codeEditor');
    const outputDiv = document.getElementById('output');
    const runBtn = document.getElementById('runBtn');
    const clearBtn = document.getElementById('clearBtn');
    const clearOutputBtn = document.getElementById('clearOutput');
    const speakBtn = document.getElementById('speakBtn');
    const themeToggle = document.getElementById('themeToggle');

    // Initialize compiler
    const compiler = new DurhamCompiler();
    
    // Text-to-Speech setup
    let isSpeaking = false;

    // Helper function to escape HTML
    function escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    // Theme toggle
    let isDarkMode = localStorage.getItem('darkMode') === 'true';
    if (isDarkMode) {
        document.documentElement.setAttribute('data-theme', 'dark');
        themeToggle.textContent = '☀️ Light Mode';
    }

    themeToggle.addEventListener('click', function() {
        isDarkMode = !isDarkMode;
        if (isDarkMode) {
            document.documentElement.setAttribute('data-theme', 'dark');
            themeToggle.textContent = '☀️ Light Mode';
        } else {
            document.documentElement.removeAttribute('data-theme');
            themeToggle.textContent = '🌙 Dark Mode';
        }
        localStorage.setItem('darkMode', isDarkMode);
    });

    // Run code
    runBtn.addEventListener('click', function() {
        const code = codeEditor.value.trim();
        
        if (!code) {
            outputDiv.innerHTML = '<span class="error">⚠️ Please write some code first!</span>';
            return;
        }

        // Visual feedback
        runBtn.classList.add('running');
        runBtn.textContent = '⏳ Running...';
        outputDiv.innerHTML = '<span style="color: #666;">Compiling...</span>';

        // Run compiler (with small delay for visual feedback)
        setTimeout(function() {
            try {
                const result = compiler.compile(code);
                
                if (result.success) {
                    if (result.output) {
                        // Escape HTML and preserve formatting
                        const escapedOutput = escapeHtml(result.output);
                        outputDiv.textContent = escapedOutput;
                    } else {
                        outputDiv.innerHTML = '<span class="success">✓ Program executed successfully (no output)</span>';
                    }
                } else {
                    const escapedError = escapeHtml(result.output);
                    outputDiv.innerHTML = `<span class="error">❌ ${escapedError}</span>`;
                }
            } catch (error) {
                outputDiv.innerHTML = `<span class="error">❌ Unexpected error: ${escapeHtml(error.message)}</span>`;
            } finally {
                runBtn.classList.remove('running');
                runBtn.textContent = '▶️ Run Code';
            }
        }, 300);
    });

    // Clear editor
    clearBtn.addEventListener('click', function() {
        if (codeEditor.value && !confirm('Are you sure you want to clear the code?')) {
            return;
        }
        codeEditor.value = '';
        codeEditor.focus();
    });

    // Clear output
    clearOutputBtn.addEventListener('click', function() {
        outputDiv.innerHTML = '';
    });

    // Text-to-Speech
    speakBtn.addEventListener('click', function() {
        const outputText = outputDiv.textContent.trim();
        
        if (!outputText) {
            alert('⚠️ No output to speak! Run some code first.');
            return;
        }

        // Check if browser supports speech synthesis
        if (!('speechSynthesis' in window)) {
            alert('❌ Sorry, your browser does not support text-to-speech.');
            return;
        }

        // Stop if already speaking
        if (isSpeaking) {
            window.speechSynthesis.cancel();
            speakBtn.textContent = '🔊 Speak Output';
            speakBtn.classList.remove('speaking');
            isSpeaking = false;
            return;
        }

        // Clean output text (remove error/success symbols)
        let cleanText = outputText
            .replace(/^[⚠️❌✓✅]+\s*/gm, '') // Remove symbols
            .trim();

        // Create speech utterance
        const utterance = new SpeechSynthesisUtterance(cleanText);
        
        // Configure speech
        utterance.rate = 1.0;  // Speed (0.1 to 10)
        utterance.pitch = 1.0; // Pitch (0 to 2)
        utterance.volume = 1.0; // Volume (0 to 1)
        
        // Try to use a high-quality voice
        const voices = window.speechSynthesis.getVoices();
        const preferredVoice = voices.find(voice => 
            voice.lang.startsWith('en') && voice.name.includes('Google')
        ) || voices.find(voice => voice.lang.startsWith('en'));
        
        if (preferredVoice) {
            utterance.voice = preferredVoice;
        }

        // Update button state
        speakBtn.textContent = '⏸️ Stop Speaking';
        speakBtn.classList.add('speaking');
        isSpeaking = true;

        // Event handlers
        utterance.onend = function() {
            speakBtn.textContent = '🔊 Speak Output';
            speakBtn.classList.remove('speaking');
            isSpeaking = false;
        };

        utterance.onerror = function(event) {
            console.error('Speech synthesis error:', event);
            speakBtn.textContent = '🔊 Speak Output';
            speakBtn.classList.remove('speaking');
            isSpeaking = false;
            alert('❌ Error speaking text: ' + event.error);
        };

        // Speak!
        window.speechSynthesis.speak(utterance);
    });

    // Load voices (needed for some browsers)
    if ('speechSynthesis' in window) {
        window.speechSynthesis.onvoiceschanged = function() {
            window.speechSynthesis.getVoices();
        };
    }

    // Keyboard shortcuts
    codeEditor.addEventListener('keydown', function(e) {
        // Ctrl/Cmd + Enter to run
        if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
            e.preventDefault();
            runBtn.click();
        }

        // Tab key for indentation
        if (e.key === 'Tab') {
            e.preventDefault();
            const start = this.selectionStart;
            const end = this.selectionEnd;
            const value = this.value;
            
            this.value = value.substring(0, start) + '    ' + value.substring(end);
            this.selectionStart = this.selectionEnd = start + 4;
        }
    });

    // Auto-save code to localStorage
    let saveTimeout;
    codeEditor.addEventListener('input', function() {
        clearTimeout(saveTimeout);
        saveTimeout = setTimeout(function() {
            localStorage.setItem('durhamCode', codeEditor.value);
        }, 1000);
    });

    // Load saved code
    const savedCode = localStorage.getItem('durhamCode');
    if (savedCode) {
        codeEditor.value = savedCode;
    }

    // Welcome message
    if (!savedCode) {
        outputDiv.innerHTML = `<span class="success">
🎓 Welcome to Durham Programming Language!

Click "Load Example" to see sample programs, or start coding!

Tips:
• Use Ctrl/Cmd + Enter to run code
• All numbers use college names (butler=0, chads=1, etc.)
• Multi-digit numbers use commas: marys,marys = 22
• Try the examples to learn the syntax!

Ready to code? 🚀
</span>`;
    }

    // Add syntax highlighting hints on hover for college names
    const collegeNames = [
        'butler', 'chads', 'marys', 'collingwood', 'johns', 'castle',
        'cuths', 'trevs', 'aidans', 'snow', 'grey', 'stephenson',
        'hatfield', 'hildbede', 'south', 'vanmildert', 'ustinov'
    ];

    codeEditor.addEventListener('mousemove', function(e) {
        const word = getWordAtPosition(this, e);
        if (word && collegeNames.includes(word.toLowerCase())) {
            const collegeMap = {
                'butler': 0, 'chads': 1, 'marys': 2, 'collingwood': 3,
                'johns': 4, 'castle': 5, 'cuths': 6, 'trevs': 7,
                'aidans': 8, 'snow': 9, 'grey': 10, 'stephenson': 11,
                'hatfield': 12, 'hildbede': 13, 'south': 14,
                'vanmildert': 15, 'ustinov': 16
            };
            this.title = `${word} = ${collegeMap[word.toLowerCase()]}`;
        } else {
            this.title = '';
        }
    });

    function getWordAtPosition(textarea, event) {
        const rect = textarea.getBoundingClientRect();
        const x = event.clientX - rect.left;
        const y = event.clientY - rect.top;
        
        // This is a simplified version - you might want a more robust implementation
        const lines = textarea.value.split('\n');
        const lineHeight = parseFloat(getComputedStyle(textarea).lineHeight);
        const lineIndex = Math.floor((y + textarea.scrollTop) / lineHeight);
        
        if (lineIndex < 0 || lineIndex >= lines.length) return null;
        
        const line = lines[lineIndex];
        const words = line.split(/\s+/);
        
        for (const word of words) {
            if (word.match(/^[a-z]+$/i)) {
                return word;
            }
        }
        
        return null;
    }

    // Add copy to clipboard functionality for output
    outputDiv.addEventListener('dblclick', function() {
        const text = this.textContent;
        if (text) {
            navigator.clipboard.writeText(text).then(function() {
                const originalHTML = outputDiv.innerHTML;
                outputDiv.innerHTML = '<span class="success">✓ Output copied to clipboard!</span>';
                setTimeout(function() {
                    outputDiv.innerHTML = originalHTML;
                }, 2000);
            });
        }
    });

    console.log('🎓 Durham Language Web Compiler loaded successfully!');
    console.log('Made with ❤️ by Wai Yan Moe Hein');
});
