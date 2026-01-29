.DEFAULT_GOAL := help

DOCKER_COMPOSE_FILE = docker-compose.yml
MYSQL84_DEV = mysql_belgium_days2026-devenv-8.4.6-1
MYSQL84_TEST =  mysql_belgium_days2026-mysql8.4-1
MYSQL_PLUGIN_OUTPUT_DIR = /development/mysql/bld/plugin_output_directory
BUILD_OUTPUT = ./artifacts


help:
	@grep -E '^[a-zA-Z0-9_.-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-11s\033[0m %s\n", $$1, $$2}'

build: ## Build the docker environment
	docker compose -f $(DOCKER_COMPOSE_FILE) build --pull

start: ## Start the docker environment
	- docker compose -f $(DOCKER_COMPOSE_FILE) up --pull never -d

stop: ## Stop the docker environment
	- docker compose -f $(DOCKER_COMPOSE_FILE) down

rm: stop  ## Remove the docker environment
	docker compose -f $(DOCKER_COMPOSE_FILE) rm --force -v

devshell: ## Execute a shell inside the container to build against MySQL 8.4 in interactive mode
	- docker exec -it $(MYSQL84_DEV) sh

testshell: ## Execute a shell inside the test container running MySQL 8.4 in interactive mode
	- docker exec -it $(MYSQL84_TEST) sh


compile: ## Compile using MySQL 8.4 the plugin and copy it to the host
	docker exec $(MYSQL84_DEV) sh -c "make -j4 mysqldays"
	VERSION=$$(docker exec $(MYSQL84_DEV) awk -F= '/MYSQL_VERSION_(MAJOR|MINOR|PATCH)/ {a=(a? a "." : "") $$2} END{print a}' VERSION.dep); \
	echo "Version: $$VERSION"; \
	mkdir -p $(BUILD_OUTPUT)/$$VERSION; \
	docker cp $(MYSQL84_DEV):$(MYSQL_PLUGIN_OUTPUT_DIR)/mysqldays.so $(BUILD_OUTPUT)/$$VERSION/mysqldays.so

