# Quick Deploy to GitHub Pages

## Method 1: Using GitHub Web Interface

1. Go to your repository on GitHub
2. Click on "Settings" tab
3. Scroll down to "Pages" section
4. Under "Source", select your branch (usually `main`)
5. Under "Folder", select `/ (root)` if web files are in root, or `/web` if in web folder
6. Click "Save"
7. Wait a few minutes - your site will be live at:
   `https://waiyanmar17.github.io/assembly-coding/`

## Method 2: Using Git Commands

```bash
# If web folder is in root
git add web/
git commit -m "Add web compiler for Durham Language"
git push origin main

# Then enable GitHub Pages in Settings → Pages
```

## Method 3: Separate gh-pages Branch (Recommended)

```bash
# Create and switch to gh-pages branch
git checkout --orphan gh-pages

# Remove all files
git rm -rf .

# Copy web files to root
cp -r web/* .

# Add and commit
git add .
git commit -m "Deploy Durham Language web compiler"

# Push to GitHub
git push origin gh-pages

# Switch back to main branch
git checkout main
```

Then in GitHub Settings → Pages, select `gh-pages` branch and `/ (root)` folder.

## Method 4: Quick Deploy with Netlify

1. Go to https://app.netlify.com/drop
2. Drag and drop your `web` folder
3. Get instant live URL!

## Method 5: Vercel Deploy

```bash
# Install Vercel CLI
npm i -g vercel

# Navigate to web folder
cd web

# Deploy
vercel

# Follow prompts
```

## Custom Domain (Optional)

In GitHub Pages settings, you can add a custom domain like:
`durham-lang.com`

Just add a CNAME file with your domain name.

## Testing Locally Before Deploy

```bash
# Using Python (if installed)
cd web
python -m http.server 8000

# Or use start-server.bat
# Then open: http://localhost:8000
```

## Troubleshooting

**404 Error?**
- Make sure files are in the correct folder
- Check that index.html exists
- Wait a few minutes for GitHub Pages to build

**CSS/JS not loading?**
- Check file paths are relative (./styles.css, not /styles.css)
- Verify all files are committed and pushed

**Site not updating?**
- Clear browser cache (Ctrl+Shift+R)
- Check GitHub Actions tab for build status
- Wait ~5 minutes for changes to propagate

## Success!

Once deployed, share your link:
`https://waiyanmar17.github.io/assembly-coding/`

Perfect for hackathon demos! 🎉
