#!/usr/bin/env bash

SCRIPT_DIR="$(dirname $(readlink -e "$0"))"
PROJECT_DIR="$(readlink -e "$SCRIPT_DIR/..")"
TARGET="$(basename "$PROJECT_DIR")"

CONFIG_DIR="$(readlink -e "$PROJECT_DIR/../../_config/openocd_cfg")"

openocd -f "$CONFIG_DIR/stlink-v2.cfg" \
  -f "$CONFIG_DIR/stm32f4x.cfg" \
  -c "program $PROJECT_DIR/$TARGET.elf verify reset"