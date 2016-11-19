@echo off
copy /y p0492-cpp17-filesystem-nb-comments.html ..\dot16-gh-pages\d0492.html
pushd ..\dot16-gh-pages
call git commit -a -m "auto commit"
call git push origin gh-pages
popd
echo "URL: https://beman.github.io/dot16/d0492.html"
