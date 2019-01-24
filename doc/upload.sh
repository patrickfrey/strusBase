git checkout gh-pages
cd doc/
git pull origin master
git rm doxygen/html/*.*
doxygen doxygen.conf 
git add doxygen/html/*.*
git status
git commit -m "update interface documentation to 0.16"
git push origin gh-pages 
git branch
git checkout master 
