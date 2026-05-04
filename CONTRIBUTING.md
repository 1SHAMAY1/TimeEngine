# Contributing to TimeEngine

First off, thank you for considering contributing to TimeEngine! It's people like you who make it a great tool for everyone.

## Code of Conduct

By participating in this project, you agree to abide by our Code of Conduct (standard Contributor Covenant).

## How Can I Contribute?

### Reporting Bugs
- Use the **GitHub Issue Tracker**.
- Check if the issue has already been reported.
- Include steps to reproduce, expected behavior, and actual behavior.

### Suggesting Enhancements
- Open a **Feature Request** issue.
- Describe the use case and why this feature would be beneficial.

### Pull Requests
1. Fork the repo and create your branch from `main`.
2. If you've added code that should be tested, add tests.
3. If you've changed APIs, update the documentation.
4. Ensure the test suite passes.
5. Make sure your code follows the existing style (`.clang-format`).

## Development Setup

1. **Clone the repository**:
   ```bash
   git clone --recursive https://github.com/1SHAMAY1/TimeEngine.git
   ```
2. **Generate Project Files**:
   Run `Scripts/GenerateProjectFiles.bat` on Windows.
3. **Build**:
   Open `TimeEngine.sln` in Visual Studio 2022 and build the `Sandbox` project.

## Code Style

We use `.clang-format` to maintain consistent code style. Please run clang-format on your changes before submitting a PR.

- **Naming**: PascalCase for Classes, camelCase for variables, SCREAMING_SNAKE_CASE for macros.
- **Headers**: Use `#pragma once`.
- **Indentation**: 4 spaces.

## Core Focus: 2D Excellence & Time Manipulation

Contributions that advance the **Inbuilt 2D Sprite Editor** or the **Time Manipulation** system (deterministic simulation, state snapshots, time-reversal) are highly prioritized. We aim to be the premier C++ solution for deterministic 2D development. See [ROADMAP.md](ROADMAP.md) for details.
