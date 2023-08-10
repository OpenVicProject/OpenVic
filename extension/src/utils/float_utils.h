class FloatUtils {
public:
	static constexpr int round_to_int(double num) {
		return (num > 0.0) ? (num + 0.5) : (num - 0.5);
	}

	static constexpr int64_t round_to_int64(double num) {
		return (num > 0.0) ? (num + 0.5) : (num - 0.5);
	}
};