/*
 * This file is part of musicpaused - music autopausing daemon
 * Copyright (C) 2012 Maxim A. Mikityanskiy - <maxtram95@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dbus/dbus.h>
#include <stdio.h>
#include <string.h>

static DBusError error;
static DBusConnection *connection;
static DBusConnection *session_connection;

static void process_event(const char *player, const char *codename, int state);

int mp_dbus_init()
{
	dbus_error_init(&error);

	// Connect
	connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	if (dbus_error_is_set(&error)) {
		fprintf(stderr, "DBus: dbus_bus_get: %s\n", error.message);
		dbus_error_free(&error);
		return 0;
	}
	if (connection == NULL) {
		fprintf(stderr, "DBus: connection to system bus failed\n");
		dbus_error_free(&error);
		return 0;
	}
	
	session_connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
	if (dbus_error_is_set(&error)) {
		fprintf(stderr, "DBus: dbus_bus_get: %s\n", error.message);
		dbus_connection_close(connection);
		dbus_error_free(&error);
		return 0;
	}
	if (session_connection == NULL) {
		fprintf(stderr, "DBus: connection to session bus failed\n");
		dbus_connection_close(connection);
		dbus_error_free(&error);
		return 0;
	}

	// Connect to jacklistener
	dbus_bus_add_match(connection, "type='signal',interface='org.ude.jacklistener'", &error);
	dbus_connection_flush(connection);
	if (dbus_error_is_set(&error)) {
		fprintf(stderr, "DBus: dbus_bus_add_match: %s\n", error.message);
		dbus_connection_close(connection);
		dbus_connection_close(session_connection);
		dbus_error_free(&error);
		return 0;
	}

	return 1;
}

void mp_dbus_fini()
{
	dbus_connection_close(connection);
	dbus_connection_close(session_connection);
	dbus_error_free(&error);
}

int mp_dbus_call_method(const char *server, const char *path, const char *interface, const char *name)
{
	dbus_uint32_t serial = 0;

	// Create method call
	DBusMessage *msg = dbus_message_new_method_call(server, path, interface, name);
	if (msg == NULL) {
		fprintf(stderr, "DBus: error creating method call\n");
		return 0;
	}

	// Call method
	if (!dbus_connection_send(session_connection, msg, &serial)) {
		fprintf(stderr, "DBus: error calling method\n");
		return 0;
	}

	dbus_connection_flush(session_connection);

	// Delete method call
	dbus_message_unref(msg);

	return 1;
}

void mp_dbus_listen_events(const char *player)
{
	for (;;) {
		// Event loop
		dbus_connection_read_write(connection, -1);
		DBusMessage *msg;
		while ((msg = dbus_connection_pop_message(connection)) != NULL) {
			// Filter signals
			if (dbus_message_is_signal(msg, "org.ude.jacklistener", "statechanged")) {
				DBusMessageIter args;
				if (!dbus_message_iter_init(msg, &args)) {
					goto final;
				}

				// Get first argument
				if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING) {
					goto final;
				}
				const char *str;
				dbus_message_iter_get_basic(&args, &str);
				
				// Get second argument
				if (!dbus_message_iter_next(&args)) {
					goto final;
				}
				if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_UINT32) {
					goto final;
				}
				int value;
				dbus_message_iter_get_basic(&args, &value);

				// Process them
				process_event(player, str, value);
			}
final:
			dbus_message_unref(msg);
		}
	}
}

static void process_event(const char *player, const char *codename, int state)
{
	if (state == 0) {
		if (!strcmp(codename, "headphone")) {
			printf("Pausing %s\n", player);
			char server[200];
			snprintf(server, sizeof(server), "org.mpris.MediaPlayer2.%s", player);
			mp_dbus_call_method(server, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", "Pause");
		}
	}
}
