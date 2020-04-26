---
layout: default
title: Development / Testing Guide
---

# Development and Testing Guide

## What you need to know

If you are a developer, here is what you need to know about changing code:

* The github is where all code lives and how code is to be distributed

* All official code lives on the master branch of the github.

* Every time changes are requested to be merged onto master (always via a pull request), a formal testing report must be included.

* All code to be merged to master must undergo and pass a review by a fellow collaborator.

* Any branch to be merged onto master needs to be up to date. ie: all commits of the non-master branch must be made on top of the most recent commits on master. This is a practice know as [rebasing](https://git-scm.com/book/en/v2/Git-Branching-Rebasing), and it will give us a nice code base histroy moving forward.


## Actually Developing

### Get the most recent code that works

You must first get the most recent code that works.

* Clone the repo

`git clone git@github.com:CSSALTlab/Open_Source_Ventilator.git`

*NOTE: if you have already cloned the repo, you do not need to do this*


* Navigate to the root of the repo

*NOTE: if you just cloned you can use* `cd Open_Source_Ventilator`

* Fetch the repository's branches (this download will suffix remote branches with `origin/` so your local branches are safe.)

`git fetch origin`

* Checkout your local master branch

`git checkout master`

* Reset your local version of the master branch to match what is on the github

**NOTE: This will delete all diffrences between your local master branch and the remote master branch.**

`git reset origin/master --hard`

### Making changes

* Checkout a new branch

`git checkout -b <my-branch-name>`

* Make some changes to the code testing along the way

* Once you are pleased with your changes

* Review your changes

  * Get a list of exatly the files that you changed

`git status`

  * See exactly how you changed those files

`git diff`

* Add all the files you changed

  `git add <file-names>`

* Make sure you are not on the master branch

`git status`

  * If you are on the master branch

`git checkout -b <my-branch name>`

* Commit your changes to your current branch with a message about what you did

`git commit -m "<i-fixed-bug-x>"`

* STOP EDITING CODE FILES

### Conduct a Full Formal Test

**NOTE: Before conducting a test, make sure your code is uptodate with master with** `git rebase master` **make sure to resolve any merge conflicts and do some more informal tests to make sure your code does indeed work as you think it should.**

* make sure you are not still in the middle of a merge / rebase with

`git status`

* Go go thourgh and test all functions listed in out [Testing Report](./blank_testing_doc.html) Filling in the boxes as you go.
**NOTE Do not refresh the page, it will delete all yourt entered values**


* If you want to add a test / change the testing report document
  * edit the testing document source file under `docs/testing/blank_testing_doc.md` to reflect the new test. 
  * Make sure to follow the steps from the [guide on adding documentation](../AddingDocumentation.html)

**NOTE: make sure to include the changes to the documentation on this branch. ie: do not checkout a separate branch for editing documentation in this instance, since the change in documentation is to go with the changes in the code already commited on this branch** 

  * Once you are happy with the changes to the testing report, commit them with

`git add <path-to-testing-report>`

`git commit -m "<i-added-x-to-testing-report>"`

  * launch the local jekyll server (instructions ini the [guide on adding documentation](../AddingDocumentation.html))

  * fill out the fillable text docs on the Blank Testing Report page from the local jekyll server (so your added tests show up)

  * download the site as html into your downloads folder

  * kill the jekyll server with `Ctrl + c`

* If you do not want to change the testing document

  * see the [blank testing report](./blank_testing_report_doc.md)

  * fill out the text boxes with your test results

  * download the webpage as html

  * add the testing report in markdown format under `docs/tests/`

  * Commit the test to this branch

`git add <path-to-my-test>`

`git commit -m <executed-test-x>`


### Request your changes be added to master branch

This is known in the business as a `Pull Request`

* push your local development branch to the github
**NOTE: make sure you are not on master branch with** `git status`

`git push origin <my-branch-name> -u`

* go to [the github](https://github.com/CSSALTlab/Open_Source_Ventilator)

* click on `Pull Requests`

* click `New Pull Request`

* select your branch under the compare tab

* select someone to review your propsed changes to double check your work. (Nessesary even after a foormal test)


