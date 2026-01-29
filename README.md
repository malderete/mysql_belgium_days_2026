## What is included?

- MySQL 8.4
- MySQL 8.4 devenv to compile
- MySQLd_exporter
- Prometheus
- Grafana


## Makefile

The [Makefile](./Makefile) has a set of targets which are intended to help to manage this project.

```
❯ make
build       Build the docker environment
compile     Compile using MySQL 8.4 the plugin and copy it to the host
devshell    Execute a shell inside the container to build against MySQL 8.4 in interactive mode
rm          Remove the docker environment
start       Start the docker environment
stop        Stop the docker environment
testshell   Execute a shell inside the test container running MySQL 8.4 in interactive mode
❯
```


## Scrape MySQLd exporter

Go to `http://localhost:9104/`


## Accesing Prometheus

Go to `http://localhost:9090/`


## Accesing Grafana

Go to `http://localhost:3000/`
