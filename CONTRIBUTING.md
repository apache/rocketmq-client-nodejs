## How To Contribute

We are always very happy to have contributions, whether for trivial cleanups or big new features. We want to have high quality, well documented codes for Node.js client SDK.

Nor is code the only way to contribute to the project. We strongly value documentation, integration with other project, and gladly accept improvements for these aspects.

## Submission Guidelines

### Submitting an Issue

Before you submit an issue, please search the issue tracker, maybe an issue for your problem already exists and the discussion might inform you of workarounds readily available.

We want to fix all the issues as soon as possible, but before fixing a bug we need to reproduce and confirm it. In order to reproduce bugs, we will systematically ask you to provide a minimal reproduction. Having a minimal reproducible scenario gives us a wealth of important information without going back & forth to you with additional questions.

A minimal reproduction allows us to quickly confirm a bug (or point out a coding problem) as well as confirm that we are fixing the right problem.

We will be insisting on a minimal reproduction scenario in order to save maintainers time and ultimately be able to fix more bugs. Interestingly, from our experience users often find coding problems themselves while preparing a minimal reproduction. We understand that sometimes it might be hard to extract essential bits of code from a larger code-base but we really need to isolate the problem before we can fix it.

Unfortunately, we are not able to investigate / fix bugs without a minimal reproduction, so if we don't hear back from you we are going to close an issue that doesn't have enough info to be reproduced.

### Submitting a Pull Request (PR)

Before you submit your Pull Request (PR) consider the following guidelines:

1. Search [GitHub](https://github.com/apache/rocketmq-client-nodejs/pulls) for an open or closed PR that relates to your submission. You don't want to duplicate effort.
2. Be sure that an issue describes the problem you're fixing, or documents the design for the feature you'd like to add. Discussing the design up front helps to ensure that we're ready to accept your work.
3. Fork the apache/rocketmq-client-nodejs repo.
4. Make your changes in a new git branch:
    ```
    $ git checkout -b your-fix-branch master    
    ```
5. Create your patch, **including appropriate test cases.**
6. Follow our [Coding Rules](#rules).
7. Run the full test suite and ensure that all tests passes.
8. Commit your changes using a descriptive commit message that follows our commit message conventions.
    ``` 
    $ git commit -a
    ```
9. Push your branch to GitHub:
    ```
    $ git push origin your-fix-branch
    ```
10. In GitHub, send a pull request to `rocketmq-client-nodejs:master`.
  + If we suggest changes then:
    - Make the required updates.
    - Re-run the test suites to ensure tests are still passing.

That's all. Thanks for contributing!

### Coding Rules

To ensure consistency throughout the source code, keep these rules in mind as you are working:

+ All features or bug fixes must be tested by one or more specs (unit-tests).
+ We follow [eslint-config-rocketmq-style](https://www.npmjs.com/package/eslint-config-rocketmq-style) for ESLint.

### Commit Message Guidelines

Our commit message is inspired from [Angular's Commit Message Format](https://github.com/angular/angular/blob/master/CONTRIBUTING.md#-commit-message-guidelines).

#### Commit Message Format

Each commit message consists of a header, a body and a footer. The header has a special format that includes a type, a scope and a subject:

```
<type>(<scope>): <subject>
<BLANK LINE>
<body>
<BLANK LINE>
<footer>
```

The **header** is mandatory and the **scope** of the header is optional.

Any line of the commit message cannot be longer 72 characters! This allows the message to be easier to read on GitHub as well as in various git tools.

The footer should contain a [closing reference](https://help.github.com/articles/closing-issues-using-keywords/) to an issue if any.

Samples:

```
chore(package): bump version to 1.0.0.

Close #123
```

##### Type

Must be one of the following:

+ **chore**: Changes that affect the build system or external dependencies (example scopes: gulp, broccoli, npm)
+ **ci**: Changes to our CI configuration files and scripts (example scopes: Travis, Circle, BrowserStack, SauceLabs)
+ **docs**: Documentation only changes
+ **feat**: A new feature
+ **fix**: A bug fix
+ **perf**: A code change that improves performance
+ **refactor**: A code change that neither fixes a bug nor adds a feature
+ **style**: Changes that do not affect the meaning of the code (white-space, formatting, missing semi-colons, etc)
+ **test**: Adding missing tests or correcting existing tests
