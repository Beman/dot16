copy /y any-proposal.html ..\dot16-gh-pages
pushd ..\dot16-gh-pages
call git commit -a -m "auto commit"
call git push origin gh-pages
popd
