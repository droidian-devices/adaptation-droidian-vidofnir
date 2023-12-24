/*
 * Copyright (C) 2024 Bardia Moshiri
 * SPDX-License-Identifier: GPL-3.0+
 * Author: Bardia Moshiri <fakeshell@bardia.tech>
 */

#include <dbus/dbus.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void write_to_file(const char *file_path, int value) {
    int fd = open(file_path, O_WRONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    char buffer[10];
    sprintf(buffer, "%d", value);
    if (write(fd, buffer, strlen(buffer)) == -1) {
        perror("Error writing to file");
    }

    close(fd);
}

void handle_properties_changed(DBusMessage *msg) {
    DBusMessageIter args, dict_entry, dict_val;
    char *interface_name;
    char *property_name;
    int new_brightness;

    if (!dbus_message_iter_init(msg, &args)) return;

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) return;
    dbus_message_iter_get_basic(&args, &interface_name);

    dbus_message_iter_next(&args);
    if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&args)) return;

    dbus_message_iter_recurse(&args, &dict_entry);
    while (dbus_message_iter_get_arg_type(&dict_entry) != DBUS_TYPE_INVALID) {
        dbus_message_iter_recurse(&dict_entry, &dict_val);

        if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&dict_val)) return;
        dbus_message_iter_get_basic(&dict_val, &property_name);

        if (strcmp(property_name, "Brightness") == 0) {
            dbus_message_iter_next(&dict_val);

            DBusMessageIter variant_val;
            if (DBUS_TYPE_VARIANT != dbus_message_iter_get_arg_type(&dict_val)) return;

            dbus_message_iter_recurse(&dict_val, &variant_val);
            if (DBUS_TYPE_INT32 == dbus_message_iter_get_arg_type(&variant_val)) {
                dbus_message_iter_get_basic(&variant_val, &new_brightness);
                //printf("Brightness changed to: %d\n", new_brightness);

                if (new_brightness == 1) {
                    write_to_file("/sys/class/leds/aw210xx_led/effect", 1);
                    write_to_file("/sys/class/leds/aw210xx_led/brightness", 255);
                } else if (new_brightness == 0) {
                    write_to_file("/sys/class/leds/aw210xx_led/effect", 0);
                    write_to_file("/sys/class/leds/aw210xx_led/brightness", 0);
                }
            }
        }
        dbus_message_iter_next(&dict_entry);
    }
}

int main(int argc, char **argv) {
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);

    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err) || conn == NULL) {
        fprintf(stderr, "Failed to get a DBus connection: %s\n", err.message);
        dbus_error_free(&err);
        exit(EXIT_FAILURE);
    }

    dbus_bus_add_match(conn, "type='signal',interface='org.freedesktop.DBus.Properties',path='/org/droidian/Flashlightd',member='PropertiesChanged'", &err);
    dbus_connection_flush(conn);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Failed to add DBus match: %s\n", err.message);
        dbus_error_free(&err);
        dbus_connection_unref(conn);
        exit(EXIT_FAILURE);
    }

    while (true) {
        dbus_connection_read_write_dispatch(conn, -1);

        DBusMessage *msg = dbus_connection_pop_message(conn);

        if (msg) {
            if (dbus_message_is_signal(msg, "org.freedesktop.DBus.Properties", "PropertiesChanged")) {
                handle_properties_changed(msg);
            }
            dbus_message_unref(msg);
        }
    }

    dbus_connection_unref(conn);
    return 0;
}
