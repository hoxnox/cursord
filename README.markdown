<!-- @author     Merder Kim <hoxnox@gmail.com>  -->
<!-- @date       2012-06-01 12:32:54 -->
<!-- @copyright  Merder Kim  -->
<!-- BSD License -->

Intro
=====

**Cursord** is a server, consequentially returns data from the *source* as the answer to the client
query.

The source can be a file, database or some specific generator.

This server written to simplify distributed computing organization and often used as task source. If
we think in Consumer-Producer model, cursord is a Producer with built in message queue.

This instrument is very lightweight and simple to use.

Usage example:

Suppose we have database, where task data are stored. We can launch cursord like this:

	./cursord -i"username='admin' dbname='data' password='mypass'" "SELECT some_data,
	some_addition_data FROM one_table, second_table WHERE some complex expression"

Now we have the service, which get us next data task as the response to GET query. So we can launch
N separated processes, getting data and doing tasks. As the result, we can manage tasks solving and
get statistic about whole process. Let see simple example. Suppose users table:

	MAILS

	username mail
	======== ================
	John     john@example.com
	Peter    peter@example.com
	Harry    harry@example.com
	Thomas   thomas@example.com

We want to launch 3 servers, which would send "Hello, %username%!" to each user. And we expect that
some new servers will be at our disposal soon. Everything is simple, launch cursord:

	./cursord --credentials="~/.credentials" "SELECT username, mail FROM mails"

And execute the following code on each server:

	cursor_host = connect(cursor-host.com);
	response = cursor_host.get();
	data = split(response, 0x1E);
	sendmail(data[0], "Hello, " + data[1]+ "!");

Now to check the mailing speed, we need just send SPEED to cursord. To stop mailing, just send STOP.

You can use not only database as data source, but file (line by line), or some specific generator:

	./cursor -g fibonachi 8

which will return 13, 21, 34, ...

In short we have ready to use tasks source for distributed computation.

Building
========

cmake

Documentation
=============

doxygen in source files and some addition in `doc` directory.

Testing
=======

gtest

Crossplatform
=============

Well, we are using it on Linux with crossplatforming in mind.

History
=======

2012-11-03

Cursord 2.0: rewrite

2012-11-01
----------

Using UDP protocol. There is no guarantee of packet transmission. Commands: GET, SPEED, STOP.
Sources: PostgreSQL database, file, generators: host, hst+port.

ROADMAP: Database engine rewrite to ODBC (unixODBC). Include custom generator support. Release
         clinet libraries for some popular languages.
