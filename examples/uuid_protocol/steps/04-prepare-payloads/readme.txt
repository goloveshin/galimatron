// System messages move into their own namespace.
//
// The session is still one version 2 exchange. The hello text is "hello client version v2!".
//
// The previous GaL left the messages in the version namespace.
// This GaL marks each of them system_message. That tag stays outside the hash, so the hashes stay.
// payload_message is the mark a later payload will use. This version has no payload message.
// The uuid xslt is unchanged.
// The generation xslt writes a system_message struct in namespace system,
// writes an empty namespace payload,
// and leaves a struct without those marks in the version namespace. Version 1 stays where it was.
//
// This shows a tag outside the hash, and a namespace chosen by a mark on the message.
//
