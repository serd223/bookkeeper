# Configuration

The behavior of `bk` can be tweaked via command line options or configuration files. The [`Usage section`](./usage.md#command-line-options) covers all command line options, this section will cover the specifics of configuration files.

`bk` looks for a file named `.bk.conf` in the current working directory by default. This file is optional and parsing a config file is skipped if it doesn't exist. Alternatively, you can supply a custom path for your config file with `bk --config-path path/to/your/file`. If a custom config path is supplied and that file cannot be found/read, `bk` reports an error and exits.

A valid config file looks like this:
```ini
# Comments start with #
key=value
key1=othervalue
```
Values can be integers, floats, strings, or booleans (denoted with `true` and `false`).

Since command line flags and config options are mostly the same, you can see the [`Usage section`](./usage.md#command-line-options) for descriptions that explain what each key means.

A notable exception to this is `include_files` vs `-i`. `-i` accepts a single file and you supply multiple `-i` flags when you want to include multiple files. On the other hand, the `include_files` option accepts a comma separated list of files (like 'file1,file2,file3'). The same also applies to `schema_files` vs `-is`.

When in doubt, see [full_config.conf](../examples/full_config.conf) for a list of all valid keys.

Note that configuration files are processed _before_ command line options. This means that any command line option will override the configuration file if they change the same option.
