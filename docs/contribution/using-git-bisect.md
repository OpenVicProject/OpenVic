# Using Git Bisect

Git bisect is a very useful tool for debugging issues that you can't find the root of, specifically when you know a period in the commit history where that issue did not exist.

How git bisect works is by binary searching through the commit history marking commits as good or bad until you reach the history responsible for the issue at hand.

To start using it you must do:
```sh
git bisect start
git bisect good <GOOD COMMIT HASH>
git bisect bad <BAD COMMIT HASH>
```

The bisect will begin, it will perform a binary search and put you at a specific commit, if the commit still has the bug then run:
```sh
git bisect bad
```
If the commit doesn't have the bug then run:
```sh
git bisect good
```
When done it will stop at the responsible commit for which you can investigate to see what's causing the issue.

For more documentation on git bisect see: https://git-scm.com/docs/git-bisect