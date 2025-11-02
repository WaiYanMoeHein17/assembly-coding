# Durham Language - Web Compiler

A web-based interpreter for the Durham Programming Language, allowing you to run Durham code directly in your browser!

## Features

✨ **Live Code Execution** - Run Durham code instantly in your browser  
🎨 **Dark/Light Theme** - Toggle between themes for comfortable coding  
📚 **Example Programs** - Load pre-made examples to learn the syntax  
💾 **Auto-save** - Your code is automatically saved to browser storage  
⌨️ **Keyboard Shortcuts** - Ctrl/Cmd + Enter to run code  
📱 **Responsive Design** - Works on desktop, tablet, and mobile  
🎓 **College Reference** - Built-in reference for all college names and values  

## How to Use

1. Open `index.html` in your web browser
2. Write Durham code in the editor
3. Click "▶️ Run Code" or press Ctrl/Cmd + Enter
4. View output in the right panel

## Example Programs

### Hello World
```durham
tlc begin "Hello, Durham University!" end.
```

### Variables
```durham
number x is castle.
tlc begin x end.
```

### String Concatenation
```durham
text first is begin "Hello" end.
text second is begin " World" end.
text message is first durham second.
tlc begin message end.
```

### If-Else
```durham
number x is castle.
if begin x lesser grey end front
    tlc begin "x is less than 10" end.
back else front
    tlc begin "x is greater than or equal to 10" end.
back
```

### For Loop
```durham
for begin i is butler. i lesser snow. i is i durham chads end front
    tlc begin i end.
back
```

## College Number System

| College | Value | College | Value |
|---------|-------|---------|-------|
| butler | 0 | snow | 9 |
| chads | 1 | grey | 10 |
| marys | 2 | stephenson | 11 |
| collingwood | 3 | hatfield | 12 |
| johns | 4 | hildbede | 13 |
| castle | 5 | south | 14 |
| cuths | 6 | vanmildert | 15 |
| trevs | 7 | ustinov | 16 |
| aidans | 8 | | |

**Multi-digit numbers:** Use commas to concatenate digits  
Example: `marys,marys` = 22

## Language Reference

### Operators
- **Addition:** `durham` (e.g., `x durham y`)
- **Subtraction:** `newcastle` (e.g., `x newcastle y`)
- **Multiplication:** `york` (e.g., `x york y`)
- **Division:** `edinburgh` (e.g., `x edinburgh y`)

### Comparisons
- **Less than:** `lesser` (e.g., `x lesser y`)
- **Greater than:** `greater` (e.g., `x greater y`)
- **Equals:** `equals` (e.g., `x equals y`)

### Control Flow
- **If statement:** `if begin condition end front ... back`
- **Else:** `back else front ... back`
- **For loop:** `for begin init. condition. increment end front ... back`

### Functions
```durham
function name begin params end front
    mcs begin return_value end.
back
```

## Deployment

### Local Development
Simply open `index.html` in a web browser.

### GitHub Pages
1. Push the `web` folder to your GitHub repository
2. Go to Settings → Pages
3. Select the branch and `/web` folder
4. Your site will be live at `https://yourusername.github.io/repository-name/`

### Netlify/Vercel
1. Drag and drop the `web` folder to Netlify or Vercel
2. Your site will be deployed instantly

## Browser Compatibility

Works on all modern browsers:
- Chrome/Edge (recommended)
- Firefox
- Safari
- Opera

## Technical Details

The web compiler is a pure JavaScript interpreter that:
- Tokenizes Durham source code
- Parses statements and expressions
- Executes code in a sandboxed environment
- Provides immediate feedback

No server-side processing required - everything runs in your browser!

## Limitations

- No file I/O operations
- No multi-threading
- Maximum 10,000 loop iterations (prevents infinite loops)
- Memory limited to browser's JavaScript heap

## Tips for Hackathon Presentation

1. **Open with a live demo** - Show Hello World running instantly
2. **Highlight the college theme** - Explain how Durham colleges become numbers
3. **Run Fibonacci** - Shows practical algorithm implementation
4. **Demonstrate string concatenation** - Unique feature
5. **Show if-else** - Proves it's a full-featured language
6. **Mobile-friendly** - Show it works on phones too!

## Contributing

Found a bug? Want to add a feature? Contributions are welcome!

## License

MIT License - feel free to use and modify!

---

Made with ❤️ for Durham University by Wai Yan Moe Hein
