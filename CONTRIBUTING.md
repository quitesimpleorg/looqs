# looqs - Contributing
Contributions are welcome, please use the following guidelines. 

## Repository
The github repository https://github.com/quitesimpleorg/looqs is the primary one. Pull-Requests and Issues go there, but you can also submit your feedback there. 

Those who prefer a more classical approach can mail their patches etc. to looqs at quitesimple org. Ideally git-format patch and git send-email. 

The repository at https://gitea.quitesimple.org/crtxcr/looqs was supposed to be the main one, but these
plans are on hold for the time being. Just ignore it. 

## Pull-Requests & Rebasing
Your merge requests should be submitted against the dev branch. master branch won't be rebased. I'll try to avoid in the dev branch. I definitly rebase WIP/feature branches. 


## Commit messages
Commit messages begin with the component your change affects, e. g.  "gui:", "cli:", "shared:", followed by the class. 
Then choose an appropriate verb in present tense. Wrong: Fixed, Fixes. Correct: Fix. Make sure lines are not too long, 
I personally go by gut feeling in this matter. 

If your change has an issue, link it at the end: Closes: https://github.com/quitesimpleorg/looqs/issues/1

Example: "shared: Indexer: Use WildcardMatcher to ignore paths"

## License
You license your changes under the GPLv3 by contributing.
