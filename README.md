# Bren's common code and utilities

This repository includes some utilities the author uses for easier and faster development of
C++ projects.

## Followed standards

This project does its best to follow these standards, from this commit onward ;-)

* [Conventional Commits v1.0.0](https://www.conventionalcommits.org/en/v1.0.0/)
* [GitHub flow](https://docs.github.com/en/get-started/using-github/github-flow)
* [Semantic Versioning v2.0.0](https://semver.org/spec/v2.0.0.html)

## Logging

This library uses a custom logging system. It is designed for ease of use and to be simpler than the
Boost logging system. It can log to an iostream (so stdout or stderr), to syslog or to a queue so
the application can deal with the formatted log messages itself.

The [MoodyCamel Blocking Concurrent Queue](https://github.com/cameron314/concurrentqueue) is used or
the log queue. This queue was chosen to be able to queue log messages from different threads
concurrently without locking, but still allow the application to implement a blocking reader for the
log messages. It should use the `wait_dequeue_bulk()` method to process groups of log messages in a
blocking fashion. `try_dequeue_bulk()` can be used if log processing should be done in a
non-blocking manner.

The new design models all logging as static methods and variables, because the design only allows
for a single logging queue. There is no need to instantiate an object. State is kept in the static
class attributes.
