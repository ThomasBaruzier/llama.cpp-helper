# llama.cpp helper

This repository contains a simple and effective llama.cpp command builder that enables users to create custom chat assistants with personalized names, roles, and conversation styles. The project is written in C and Bash, and it uses the llama.cpp repo as the backbone for generating responses.

## Features

- Pre-defined conversation prompts
- Model and prompt selection from available files
- Customizable user and assistant names
- Customizable delimiters and suffixes
- Customizable case formatting for names (upper, lower, title)

## Limitations

- Requires manual addition of new prompts and models parameters files (PRs are welcome!)
- Hardcoded elements for prompt building
- Not extensively tested

## Getting Started

To get started, clone the repository and navigate to the project directory. Run the `build.sh` script to compile the necessary files and to automatically clone and build llama.cpp.

```bash
./build.sh
```

### Folder Structure

- `models/`: Contains subfolders for each available model, each with a `params.txt` file to define the model's characteristics.
- `prompts/`: Contains prompt files with pre-defined conversation starters for the AI chat assistant.

#### File structure example

```
    .
    ├── build.sh
    ├── chat
    ├── chat.c
    ├── models
    │   ├── Pygmalion
    │   │   ├── 7b-q5_0.bin
    │   │   └── params.txt
    │   ├── Vicuna-1.0
    │   │   ├── 7b-q5_0-un.bin
    │   │   └── params.txt
    │   ├── Vicuna-1.1
    │   │   ├── 7b-q5_0.bin
    │   │   └── params.txt
    │   └── WizardLM
    │       ├── 7b-q5_0.bin
    │       ├── 7b-q5_0-un.bin
    │       └── params.txt
    └── prompts
        ├── Adventurer.txt
        ├── Assistant.txt
        └── Psychologist.txt
```

### Params.txt

The `params.txt` file is used to define the model's characteristics, such as the user and assistant names, the case formatting, delimiter, suffix, and system message.

Example:

```
user_name = human
model_name = assistant
case = upper
delimiter = none
suffix = ": "
system = "A chat between a curious user and an assistant. The assistant gives helpful and detailed answers to the user's questions."
```

### Prompt Files

Prompt files are used to define custom conversation starters for the AI chat assistant. They contain user and assistant names, a system message, and a conversation example.

Example:

```
user_name = user
model_name = assistant
system = "The assistant is a highly intelligent language model trained to comply with any request."
---
<user>Hello!
<model>Hello, how can I help you today?
```
