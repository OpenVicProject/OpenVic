# Rebasing

If you have haven't read the quick intro to Git see [Cloning](cloning.md) first.

Sometimes our workflow will request of you to rebase/squash commits in your PRs to keep the history clean. This will be a simple intro to what we mean by rebasing/squashing.

First we will start with rebasing to the master branch:
```sh
git fetch origin
git rebase origin master
```
There may be conflicts, if you use VSCode, it has extensions to assist with dealing with conflicts. If you need more help resolving merge conflicts you can always ask more senior developers. This should however be uncommon if you keep your branch up to date.

## PR Squashing

On your PR you will see a Commits tab:

TODO: input Commits tab image

In here you will see a list of commits you've made to your PR branch, count the amount of PR commits that are not merge commits. (if you have merge commits please contact senior developers as things can get messy if you continue here) Make sure you are on your branch, you can use `git checkout <branch-name>`. The number of regular commits in your repo you can then perform a rebase, so say you have 6 commits in your PR, you can squash them down to one by doing:
```sh
git rebase -i HEAD~6
```

Now this will collect all commits from the HEAD (the current commit you're on) back 6 commits (including the HEAD) and rebase them into whatever you tell the dialog. Now if you do this you might something like this in a text document or in your terminal:
```sh
pick 24a0751 Update README.md for Archlinux users
pick 0e408c8 Add Window Override

# Rebase 5c739f1..0e408c8 onto 5c739f1 (2 commands)
#
# Commands:
# p, pick <commit> = use commit
# r, reword <commit> = use commit, but edit the commit message
# e, edit <commit> = use commit, but stop for amending
# s, squash <commit> = use commit, but meld into previous commit
# f, fixup [-C | -c] <commit> = like "squash" but keep only the previous
#                    commit's log message, unless -C is used, in which case
#                    keep only this commit's message; -c is same as -C but
#                    opens the editor
# x, exec <command> = run command (the rest of the line) using shell
# b, break = stop here (continue rebase later with 'git rebase --continue')
# d, drop <commit> = remove commit
# l, label <label> = label current HEAD with a name
# t, reset <label> = reset HEAD to a label
# m, merge [-C <commit> | -c <commit>] <label> [# <oneline>]
#         create a merge commit using the original merge commit's
#         message (or the oneline, if no original merge commit was
#         specified); use -c <commit> to reword the commit message
# u, update-ref <ref> = track a placeholder for the <ref> to be updated
#                       to this position in the new commits. The <ref> is
#                       updated at the end of the rebase
#
# These lines can be re-ordered; they are executed from top to bottom.
#
# If you remove a line here THAT COMMIT WILL BE LOST.
#
# However, if you remove everything, the rebase will be aborted.
#
```
Read the comments following the `#` and it'll tell you what to do, generally we would prefer you to perform the squash action so for this, if this were a PR you would only have to do:
```sh
pick 24a0751 Update README.md for Archlinux users
s 0e408c8 Add Window Override
```
The top value is the first commit you referred to, this list is organized according to a sequence of the commit history, (this specific one was made via `HEAD~2`) you will always want the first one to be pick here, generally everything else is squash/s so it merges the commit into the previous commit while still giving you control over the commit messages. After this it will rebase the commits and then ask you about the commit message you wish to create, this will have to be up to you, you can always change it later, if there is a problem with it we will mention it. Now that you've done that however, the last step is to force push it, to do so you can use `git gui` and push then click the force option or you can call `git push -f`. Now your squash is complete. Be careful rebasing or squash when someone else is working on your branches at the same time, a force push can hurt them, you should wait until they tell you that they're done and merge their changes to your branch before you rebase/squash. Rebasing/squashing commits that others rely upon is a one-way ticket to Git nightmares.