---
layout: default
title: Adding Documentation
---

# Adding Documentation

## What you need to know
If you are a developer wanting to add documentation to the project here is what you need to know.

* All documentation is to live on the github.

* All documentation is in [markdown](https://en.wikipedia.org/wiki/Markdown) format.

* Documenation is compiled into html by a program called [jekyll](https://jekyllrb.com/)

* The Documentation markdown files on the master branch are automatically compiled to html by jekyll (run on a server at Github)

* Github hosts the documentation html files for free under Github pages.

* Github pages is how we will distribute all documentation about this project.

* Hopefully soon, the github will hold all information about the project.

* If you are unsure if a peice of information should go into the repository or not, it should.


**Before you proceed, make sure you have cloned the repo with** `git clone git@github.com:CSSALTlab/Open_Source_Ventilator.git`


## How to Install Jekyll

If you want to add to documentation, you need to install the markdown to html compiler (jekyll) to make sure your changes actaully build.


### Install Ruby

### Ubuntu Linux
*NOTE: this may also work on Mac*
* see if you have ruby with `ruby -v`
  * if you do not have it install it with: `sudo apt install ruby`

### Windows
* Download [Ruby](https://rubyinstaller.org/)


## Install Bundler
 * For very good reasons, install a program to manage Ruby Environment variables called `Bundler` with the command:

`gem install bundler`

## Use Bundler to install all the other things that we need
* Navigate to the root of the repository (in a termianl)
* Issue the command

`bundle install`

* You are now ready to edit Documentation!

## Edit Documentation

### Launch the local jekyll server
* **make sure you have checked out a branch, so you are not editing on master branch with** `git checkout -b <my-new-branch-name>` (use `git status` to find out what branch you currently have checked out)

* In a terminal, launch the jekyll server. This will compile the markdown documents to html in real time (every time you save a markdown file) in the exact same way as they will be compiled in the Github Pages website.

`bundle exec jekyll serve`

*NOTE: this will not work if you are editing the _config.yml file, then the local jekyll server needs to be relaunched every time you change that file*

* open the url `http://127.0.0.1:4000/docs/` in your favorite web browser

* Make a change to a file, save the file, refresh the web page, and see the change in your web browser! WOW Computers are so Cool!!!

*NOTE: if your changes are not showing up in the local jekyll server, then that means that you have entered invalid markdown. Go look at the terminal where you are running the jekyll server. It will show you what files fail to build and where. If you are still having trouble, try googling the exact error message, other people may have had the smae problem.*

### Add a Documentation file

* Make a new text file with the extention `.md` or `.markdown` in the `/docs` directory

* make sure the first lines of the file look like this
```
---
layout: default
title: My Documentation Title
---
```

* [run the local jekyll server](#launch-the-local-jekyll-server)

* Make sure to save the file.

* see the file title show up in the top bar.

*NOTE: If the file is not showing up the the top bar, look at the terminal where you are running the jekyll server to see why that file is not building*

* click on the file's title in the top bar of the web page.

* see what your documentation page looks like!

* make changes to it. (if you are new to markdown, there is a lot of good exaple in the [operator's manual](./OperatorsManual/manual.md)


## Commiting Changes
* when you are happy with your documentation
* take a look at all the files you have chenged since the last git commit on this branch with `git status`
* see how those files have changed with `git diff`
* add only the files that you meant to change with `git add <file>`
* commit those changes with `git commit -m <what-i-changed>`


## Request changes get added to master
request your changes be added to master, know in the buisness as a `Pull Request`

* push your branch to the github with `git push origin <my-branch-name> -u`
*NOTE: you must be invited as a contributor to push your branch*

* go to the [github](https://github.com/CSSALTlab/Open_Source_Ventilator) and click on `Pull Requests`
* click on `New pull request`
* select your branch
* leave a breif description of what you did.
* request someone else to review your changes and make sure you are not crazy.





