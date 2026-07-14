PYTHON = python3
SCRIPT = mqtt_temp.py
EXECUTABLE = mqtt_temp
VENV = mqtt

.ONESHELL:

all: build

build:
        bash -c "source ./bin/activate; pyinstaller --onefile $(SCRIPT);"

run:
        ./dist/$(EXECUTABLE)

install:
        bash -c "source ./bin/activate; pip3 install -r requirements.txt"

clean:
        rm -rf dist build __pycache__ *.spec

create_venv:
        python3 -m venv .

activate_venv:
        @echo "Please run 'source ./bin/activate' to activate the virtual environment"

deactivate:
        @echo "deactivate"

help:
        @echo "Available targets:"
        @echo "  all           Build the project"
        @echo "  build         Build the project"
        @echo "  run           Run the executable"
        @echo "  install       Install dependencies"
        @echo "  clean         Clean the project directory"
        @echo "  create_venv   Create a virtual environment"
        @echo "  activate_venv Activate the virtual environment"
        @echo "  deactivate    Deactivate the virtual environment"

.PHONY: all build run install clean create_venv activate_venv help

default: build