# Deployment using Docker

## Install Docker

https://www.docker.com/

## Install WSL2

Skip if deploying on linux.  
https://learn.microsoft.com/en-us/windows/wsl/install

## Provide server configs path

Create `.env` file in repo root directory and provide path to server configuration files:
```
CONFIG_PATH=C:\Path\To\Server\Configs
```

## Build the images

```
docker compose build
```

## Start services

```
docker compose up
```

# Dev environment via VS Code on Windows

Setup on Windows makes it easy to test the emu via Game Client which is available on Windows only.

There is ready to use `.devcontainer.json` [file](../.devcontainer/devcontainer.json) in `.devcontainer` directory.

Install [VSCode](https://code.visualstudio.com/).

Install _Dev Containers_ plugin: `
ms-vscode-remote.remote-containers`

`CTRL+SHIFT+P`

`> Reopen in Container`

After reopened:

`CTRL+SHIFT+P`

`> Run Task`  
`> build-dbg`

After build is completed:

`CTRL+SHIFT+P`

`> Debug: Select and Start Debugging`  
`> debugging`

