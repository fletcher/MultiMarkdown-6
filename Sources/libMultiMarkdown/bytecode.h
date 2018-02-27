
/// 9 bytes should be sufficient to cover all tokens (max is 512)

enum opcode_flags {
	OP_PRINT_SOURCE         = 1 << 10,   //!< Print the text of the token
	OP_PRINT_HEAD           = 1 << 11,   //!< Print header string before token
	OP_PRINT_TAIL           = 1 << 12,   //!< Print tail string after token
	OP_PRINT_LOCALIZED      = 1 << 13,   //!< Print localized version of token text
	OP_PAD_SINGLE           = 1 << 14,   //!< Pad with leading newline
	OP_PAD_DOUBLE           = 1 << 15,   //!< Pad with 2 leading newlines
	OP_SET_PADDING          = 1 << 16,   //!< Set padding after token
};
