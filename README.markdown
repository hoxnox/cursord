<!-- @author     Merder Kim <hoxnox@gmail.com>  -->
<!-- @date       2012-06-01 12:32:54 -->
<!-- @copyright  Merder Kim  -->
<!-- BSD License -->

Intro
=====

**Cursord** is a server, consequentially returned data from the *source* as
the answer to the client query.

The source defines by the type of cursor. Type is set as option, when
cursor starts. There are the following cursor types: file, database or some
specific generator. Each of cursor types has arguments:
	
* file - filename
* odbc - data source name, query and delimiter
* generator - generator type and its args. The common args for all
  generators are: init (initial value) and repeat (loop to head on end of
  sequence).

There are the following generators available:

* int
* ipv4

This server written to simplify distributed computing organization and
often used as task source. If we think in Consumer-Producer model, cursord
is a Producer with built in message queue.

This instrument is very lightweight and simple to use.

Usage example:

Suppose we have database, where task data are stored. We can launch cursord
like this:

	./cursord  -f odbc -a "dsn=ODBCName,query='SELECT some_data,
	some_addition_data FROM one_table, second_table WHERE some complex expression'"

Now we have the service, which get us next data task as the response to GET
query. So we can launch N separated processes, getting data and doing
tasks. As the result, we can manage tasks solving and get statistic about
whole process. Let see simple example. Suppose users table:

	MAILS

	username mail
	======== ================
	John     john@example.com
	Peter    peter@example.com
	Harry    harry@example.com
	Thomas   thomas@example.com

We want to launch 3 servers, which would send "Hello, %username%!" to each
user. And we expect that some new servers will be at our disposal soon.
Everything is simple, launch cursord:

	./cursord -t odbc -a "credentials=~/.credentials,query='SELECT username, mail FROM mails'"

And execute the following code on each server:

	cursor_host = connect(cursor-host.com);
	response = cursor_host.get();
	data = split(response, 0x1E);
	sendmail(data[0], "Hello, " + data[1]+ "!");

Now to check the mailing speed, we need just send SPEED to cursord. To stop
mailing, just send STOP.

You can use not only database as data source, but file (line by line), or
some specific generator:

	./cursord -t generator -a "name=fibbonachi,init=8"

which will return 13, 21, 34, ...

In short we have ready to use tasks source for distributed computation.

Client and the library
======================

There is client application, that can be used from command line to
communicate with the server and the library, witch can be used in client
applications.

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

License
=======

Licensed under New BSD license.

History
=======

2013-12-25
----------

1. For IPv4 generator you can restore cursord state after the server was
   stopped, even if mix is used.
2. Shared mode released. You can launch many independent copies of
   cursord, which will be share the same info.
3. Bugfixing
4. Progressinfo added to log
5. IPv4Range in file cursor feature released


2013-02-08
----------

Generators changed, now using uint32 and ipv4. IPv4 generator can mix
values.

2012-11-01
----------

Using UDP protocol. There is no guarantee of packet transmission. Commands:
GET, SPEED, STOP. Sources: PostgreSQL database, file, generators: host,
hst+port.

ROADMAP: Database engine rewrite to ODBC (unixODBC). Include custom
         generator support. Release clinet libraries for some popular
         languages.

2012-10-01
----------

Rewrite from scratch.

