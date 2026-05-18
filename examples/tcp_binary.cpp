#include <mirror/backends/binary.hpp>
#include <mirror/mirror.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

struct LoginRequest
{
    std::string   username;
    std::uint64_t session_id = 0;
};

std::array<unsigned char, 4> encode_big_endian_size(std::uint32_t size)
{
    return {
        static_cast<unsigned char>((size >> 24U) & 0xFFU),
        static_cast<unsigned char>((size >> 16U) & 0xFFU),
        static_cast<unsigned char>((size >> 8U) & 0xFFU),
        static_cast<unsigned char>(size & 0xFFU)
    };
}

std::uint32_t decode_big_endian_size(const std::array<unsigned char, 4>& size)
{
    return (static_cast<std::uint32_t>(size[0]) << 24U) |
           (static_cast<std::uint32_t>(size[1]) << 16U) |
           (static_cast<std::uint32_t>(size[2]) << 8U) |
           static_cast<std::uint32_t>(size[3]);
}

template <typename Message>
std::vector<unsigned char> make_tcp_frame(const Message& message)
{
    const auto payload = mirror::binary::write(mirror::serialize(message));
    const auto size = encode_big_endian_size(static_cast<std::uint32_t>(payload.size()));

    std::vector<unsigned char> frame;
    frame.reserve(size.size() + payload.size());
    frame.insert(frame.end(), size.begin(), size.end());
    frame.insert(frame.end(), payload.begin(), payload.end());
    return frame;
}

template <typename Message>
Message parse_tcp_frame(const std::vector<unsigned char>& frame)
{
    if(frame.size() < 4)
    {
        throw std::runtime_error{"incomplete TCP frame"};
    }

    std::array<unsigned char, 4> size_bytes{};
    std::copy_n(frame.begin(), size_bytes.size(), size_bytes.begin());

    const auto payload_size = decode_big_endian_size(size_bytes);
    if(frame.size() - size_bytes.size() != payload_size)
    {
        throw std::runtime_error{"TCP frame size mismatch"};
    }

    const auto payload = std::string_view{
        reinterpret_cast<const char*>(frame.data() + size_bytes.size()),
        payload_size
    };

    return mirror::deserialize<Message>(mirror::binary::read(payload));
}

int main()
{
    const LoginRequest request{
        .username = "fatih",
        .session_id = 42
    };

    const auto frame = make_tcp_frame(request);
    const auto decoded = parse_tcp_frame<LoginRequest>(frame);

    return decoded.username == request.username && decoded.session_id == request.session_id ? 0 : 1;
}
