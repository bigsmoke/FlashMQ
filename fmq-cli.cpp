#include <iostream>

#define FLASHMQ_CLI_DEFAULT_MSG_FORMAT "\"%t\" %m\\n"

void fmq_cli_usage()
{
    std::cout << "fmq-cli is a full-fledged MQTT client program for both interactive and non-interactive use.\n"
        << "\n"
        << "Usage:\n"
        << "    fmq-cli [options]\n"
        << "    fmq-cli [options] <url> [<payload>]\n"
        << "    fmq-cli [options] <action>...\n"
        << "    fmq-cli [options] -P\n"
        << "    fmq-cli --help|-h\n"
        << "\n"
        << "<action> can be one of:\n"
        << "    con[n[ect]] [-w [-r] [-q <qos>] -t <topic> -p <payload>] [-e <exec>]\n"
        << "    disconnect\n"
        << "    close\n"
        << "    sub[scribe] -t <topic>\n"
        << "    unsub[scribe] -t <topic>\n"
        << "    pub[lish] [-r] [-q <qos>] [-t <topic>] -p <payload> [<url>]\n"
        << "    connack|puback|pubrec|pubrel|pubcomp|pingreq|pingresp\n"
        << "    {\"type:\" \"<PACKET_TYPE>\"}\n"
        << "\n"
        << "Options:\n"
        << "    --msg-termination='\\0'|-0\n"
        << "    --msg-termination='\\n'  The default\n"
        << "    --msg-format='\"%t\" %p\\n'\n"  // Not a good idea; too restrictive for interactive use, with a default topic, for instance
        << "    -w|--will     Include a Will Message in the CONNECT payload.\n"
        << "    -t|--topic    The topic name to publish or the topic filter to subscribe to.\n"
        << "                  -t will override a topic name/filter specified as part of the <url>.\n"
        << "    -q|--qos      The quality of service for publishing or receiving the message(s). Defaults to 0.\n"
        << "                  -q will override a topic name/filter specified as part of the <url>.\n"
        << "    -p|--payload  The message payload.\n"
        << "\n"
        << "Environment variables:\n"
        << "    FMQ_CLI_URL        The MQTT server <url> as specified below.\n"
        << "    FMQ_CLI_HOST       The MQTT host to connect to. The -h option takes precedence.\n"
        << "    FMQ_CLI_PORT       The MQTT host port to connect to.\n"
        << "    FMQ_CLI_USER       The MQTT username to use for connecting.\n"
        << "    FMQ_CLI_PASS       The MQTT passthingie to use for connecting.\n"
        << "    FMQ_CLI_PASSFILE   A file path to read the MQTT passthingie from.\n"
        << "    FMQ_CLI_CAPATH\n"
        << "    FMQ_CLI_CAFILE\n"
        << "    FS                 Message field separator.\n"
        << "\n"
        << "$CLI topics:\n"
        << "    $CLI/<cliid>/\n"
        << "\n"
        << "Placeholders:\n"
        << "    <url>       Takes the form of one of:\n"
        << "                (mqtt|http)[s]://[username[:password]@]host[:port]/topic\n"
        << "    <qos>       Quality of service level: 0 (the default), 1 or 3.\n"
        << "    <in_file>   Can be a file path or STDIN|0|-\n"
        << "    <out_file>  Can be a file path, STDOUT|1 or STDERR|2.\n"
        << std::endl;
}

// By default, an input message is interpreted as just a payload.
// If no default topic has been set yet, not specifying a topic is an error.
// A topic can be specified by stating the input line/record with a single or double quote character.
// You can use as many identical quote characters as desired, as long as the topic name is ended with the same
// amount of matching quote characters.
// If the message starts with a 0, 1, or 2, that will be interpreted as the desired QoS level for that message.
//
// Debugging stuff will always be sent to stderr

int main(int argc, char **argv)
{
}
