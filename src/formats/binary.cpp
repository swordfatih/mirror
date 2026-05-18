#include <mirror/formats/binary.hpp>

#include <mirror/codec/scalar_utils.hpp>

#include <bit>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace mirror::binary
{
namespace
{

constexpr std::string_view magic = "MIRR\1";

enum class tag : std::uint8_t
{
    object = 1,
    array = 2,
    string = 3,
    character = 4,
    signed_integer = 5,
    unsigned_integer = 6,
    floating_point = 7,
    boolean = 8,
    null = 9
};

enum class numeric_encoding : std::uint8_t
{
    binary = 0,
    text = 1
};

std::uintmax_t zigzag_encode(std::intmax_t value)
{
    const auto unsigned_value = static_cast<std::uintmax_t>(value);
    return (unsigned_value << 1U) ^ static_cast<std::uintmax_t>(-(value < 0));
}

std::intmax_t zigzag_decode(std::uintmax_t value)
{
    return static_cast<std::intmax_t>((value >> 1U) ^ (~(value & 1U) + 1U));
}

template <typename Type>
bool parse_integer_text(std::string_view text, Type& output)
{
    const auto* begin = text.data();
    const auto* end = begin + text.size();
    const auto result = std::from_chars(begin, end, output);
    return result.ec == std::errc{} && result.ptr == end;
}

std::size_t checked_size(std::uintmax_t value)
{
    if(value > std::numeric_limits<std::size_t>::max())
    {
        throw std::runtime_error{"binary size is too large"};
    }
    return static_cast<std::size_t>(value);
}

class writer
{
public:
    std::string finish()
    {
        return std::move(output);
    }

    void write_header()
    {
        output.append(magic);
    }

    void write_value(const mirror::value& input)
    {
        switch(input.type)
        {
            case mirror::value::kind::object:
                write_byte(tag::object);
                write_varuint(input.fields.size());
                for(const auto& [name, child]: input.fields)
                {
                    write_string(name);
                    write_value(child);
                }
                break;
            case mirror::value::kind::array:
                write_byte(tag::array);
                write_varuint(input.elements.size());
                for(const auto& element: input.elements)
                {
                    write_value(element);
                }
                break;
            case mirror::value::kind::string:
                write_byte(tag::string);
                write_string(input.text);
                break;
            case mirror::value::kind::character:
                write_byte(tag::character);
                write_varuint(input.bits);
                write_string(input.text);
                break;
            case mirror::value::kind::signed_integer:
                write_byte(tag::signed_integer);
                write_signed_integer(input);
                break;
            case mirror::value::kind::unsigned_integer:
                write_byte(tag::unsigned_integer);
                write_unsigned_integer(input);
                break;
            case mirror::value::kind::floating_point:
                write_byte(tag::floating_point);
                write_floating_point(input);
                break;
            case mirror::value::kind::boolean:
                write_byte(tag::boolean);
                write_byte(input.boolean ? 1U : 0U);
                break;
            case mirror::value::kind::null:
                write_byte(tag::null);
                break;
        }
    }

private:
    std::string output;

    void write_byte(std::uint8_t value)
    {
        output.push_back(static_cast<char>(value));
    }

    void write_byte(tag value)
    {
        write_byte(static_cast<std::uint8_t>(value));
    }

    void write_byte(numeric_encoding value)
    {
        write_byte(static_cast<std::uint8_t>(value));
    }

    void write_varuint(std::uintmax_t value)
    {
        while(value >= 0x80U)
        {
            write_byte(static_cast<std::uint8_t>((value & 0x7FU) | 0x80U));
            value >>= 7U;
        }
        write_byte(static_cast<std::uint8_t>(value));
    }

    void write_little_endian(std::uint64_t value, std::size_t byte_count)
    {
        for(std::size_t index = 0; index < byte_count; ++index)
        {
            write_byte(static_cast<std::uint8_t>(value >> (index * 8U)));
        }
    }

    void write_string(std::string_view value)
    {
        write_varuint(value.size());
        output.append(value);
    }

    void write_signed_integer(const mirror::value& input)
    {
        write_varuint(input.bits);

        std::intmax_t parsed = 0;
        if(parse_integer_text(input.text, parsed))
        {
            write_byte(numeric_encoding::binary);
            write_varuint(zigzag_encode(parsed));
            return;
        }

        write_byte(numeric_encoding::text);
        write_string(input.text);
    }

    void write_unsigned_integer(const mirror::value& input)
    {
        write_varuint(input.bits);

        std::uintmax_t parsed = 0;
        if(parse_integer_text(input.text, parsed))
        {
            write_byte(numeric_encoding::binary);
            write_varuint(parsed);
            return;
        }

        write_byte(numeric_encoding::text);
        write_string(input.text);
    }

    void write_floating_point(const mirror::value& input)
    {
        write_varuint(input.bits);

        if(input.bits == 32)
        {
            const float parsed = mirror::codec::parse_floating_point<float>(input.text);
            write_byte(numeric_encoding::binary);
            write_little_endian(std::bit_cast<std::uint32_t>(parsed), 4);
            return;
        }
        if(input.bits == 64)
        {
            const double parsed = mirror::codec::parse_floating_point<double>(input.text);
            write_byte(numeric_encoding::binary);
            write_little_endian(std::bit_cast<std::uint64_t>(parsed), 8);
            return;
        }

        write_byte(numeric_encoding::text);
        write_string(input.text);
    }
};

class reader
{
public:
    explicit reader(std::string_view source)
        : input{source}
    {
    }

    mirror::value read_document()
    {
        if(input.size() < magic.size() || input.substr(0, magic.size()) != magic)
        {
            throw std::runtime_error{"invalid binary document"};
        }
        position = magic.size();

        auto output = read_value();
        if(position != input.size())
        {
            throw std::runtime_error{"trailing data in binary document"};
        }
        return output;
    }

private:
    std::string_view input;
    std::size_t      position = 0;
    std::size_t      last_bits = 0;

    std::uint8_t read_byte()
    {
        if(position >= input.size())
        {
            throw std::runtime_error{"unexpected end of binary document"};
        }
        return static_cast<std::uint8_t>(input[position++]);
    }

    std::uintmax_t read_varuint()
    {
        std::uintmax_t value = 0;
        unsigned       shift = 0;

        while(true)
        {
            const auto byte = read_byte();
            if(shift >= std::numeric_limits<std::uintmax_t>::digits)
            {
                throw std::runtime_error{"invalid variable-length integer"};
            }
            if(shift == std::numeric_limits<std::uintmax_t>::digits - 1U && (byte & 0x7FU) > 1U)
            {
                throw std::runtime_error{"invalid variable-length integer"};
            }

            value |= static_cast<std::uintmax_t>(byte & 0x7FU) << shift;
            if((byte & 0x80U) == 0)
            {
                return value;
            }
            shift += 7;
        }
    }

    std::uint64_t read_little_endian(std::size_t byte_count)
    {
        std::uint64_t value = 0;
        for(std::size_t index = 0; index < byte_count; ++index)
        {
            value |= static_cast<std::uint64_t>(read_byte()) << (index * 8U);
        }
        return value;
    }

    std::string read_string()
    {
        const auto size = read_varuint();
        if(size > input.size() - position)
        {
            throw std::runtime_error{"unexpected end of binary document"};
        }

        const auto checked = checked_size(size);
        auto output = std::string{input.substr(position, checked)};
        position += checked;
        return output;
    }

    numeric_encoding read_numeric_encoding()
    {
        const auto encoding = read_byte();
        if(encoding == static_cast<std::uint8_t>(numeric_encoding::binary))
        {
            return numeric_encoding::binary;
        }
        if(encoding == static_cast<std::uint8_t>(numeric_encoding::text))
        {
            return numeric_encoding::text;
        }
        throw std::runtime_error{"invalid numeric encoding"};
    }

    mirror::value read_value()
    {
        switch(static_cast<tag>(read_byte()))
        {
            case tag::object:
                return read_object();
            case tag::array:
                return read_array();
            case tag::string:
                return mirror::value::string(read_string());
            case tag::character:
                return mirror::value::character(read_string_with_bits(), last_bits);
            case tag::signed_integer:
                return read_signed_integer();
            case tag::unsigned_integer:
                return read_unsigned_integer();
            case tag::floating_point:
                return read_floating_point();
            case tag::boolean:
                return mirror::value::boolean_value(read_bool());
            case tag::null:
                return {};
        }

        throw std::runtime_error{"unknown binary value kind"};
    }

    std::string read_string_with_bits()
    {
        last_bits = static_cast<std::size_t>(read_varuint());
        return read_string();
    }

    mirror::value read_object()
    {
        mirror::value output;
        output.type = mirror::value::kind::object;

        const auto count = read_varuint();
        output.fields.reserve(checked_size(count));
        for(std::uintmax_t index = 0; index < count; ++index)
        {
            auto name = read_string();
            auto child = read_value();
            output.fields.emplace_back(std::move(name), std::move(child));
        }

        return output;
    }

    mirror::value read_array()
    {
        auto output = mirror::value::array();

        const auto count = read_varuint();
        output.elements.reserve(checked_size(count));
        for(std::uintmax_t index = 0; index < count; ++index)
        {
            output.elements.emplace_back(read_value());
        }

        return output;
    }

    mirror::value read_signed_integer()
    {
        const auto bits = static_cast<std::size_t>(read_varuint());
        if(read_numeric_encoding() == numeric_encoding::text)
        {
            return mirror::value::signed_integer(read_string(), bits);
        }

        return mirror::value::signed_integer(std::to_string(zigzag_decode(read_varuint())), bits);
    }

    mirror::value read_unsigned_integer()
    {
        const auto bits = static_cast<std::size_t>(read_varuint());
        if(read_numeric_encoding() == numeric_encoding::text)
        {
            return mirror::value::unsigned_integer(read_string(), bits);
        }

        return mirror::value::unsigned_integer(std::to_string(read_varuint()), bits);
    }

    mirror::value read_floating_point()
    {
        const auto bits = static_cast<std::size_t>(read_varuint());
        if(read_numeric_encoding() == numeric_encoding::text)
        {
            return mirror::value::floating_point(read_string(), bits);
        }

        if(bits == 32)
        {
            const auto raw = static_cast<std::uint32_t>(read_little_endian(4));
            return mirror::value::floating_point(
                mirror::codec::floating_to_string(std::bit_cast<float>(raw)),
                bits
            );
        }
        if(bits == 64)
        {
            const auto raw = read_little_endian(8);
            return mirror::value::floating_point(
                mirror::codec::floating_to_string(std::bit_cast<double>(raw)),
                bits
            );
        }

        throw std::runtime_error{"unsupported binary floating-point width"};
    }

    bool read_bool()
    {
        const auto value = read_byte();
        if(value > 1)
        {
            throw std::runtime_error{"invalid boolean value"};
        }
        return value == 1;
    }
};

} // namespace

std::string write(const mirror::value& input)
{
    writer output;
    output.write_header();
    output.write_value(input);
    return output.finish();
}

mirror::value read(std::string_view input)
{
    return reader{input}.read_document();
}

} // namespace mirror::binary
