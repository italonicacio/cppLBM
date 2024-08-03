# LBM

## Build

Para buildar o projeto basta executar o build.sh na raiz do projeto com o nome da pasta onde vai ser feito o build e adicionar um dos modos.

* DEBUG
* RELEASE
* RELEASE_TESTS

```bash
./build.sh build DEBUG
```

Ao executar o build.sh ele vai deletar a pasta build e recriar para fazer um build limpo.

## Run

Para executar o projeto você pode fazer diretamente no executável na pasta build.

Mas criei um script onde ele limpa a pasta output e rebuild o projeto com cache e roda o projeto.

  Obs.: Diferente do build.sh, o run.sh depende que o nome da pasta,onde foi criado o projeto, seja build.

```bash
./run.sh
```
