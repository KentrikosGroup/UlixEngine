#pragma once
#include <ulix/path.hpp>
#include <ulix/pack.hpp>
#include <ulix/__inside_impl/log_utils.hpp>
#include <ulix/__required_libraries/wuffs/wuffs.hpp>
#include <variant>

namespace ulx::pds {
    using variantdec = std::variant<
        std::unique_ptr<wuffs_bmp__decoder>,
        std::unique_ptr<wuffs_gif__decoder>,
        std::unique_ptr<wuffs_jpeg__decoder>,
        std::unique_ptr<wuffs_nie__decoder>,
        std::unique_ptr<wuffs_netpbm__decoder>,
        std::unique_ptr<wuffs_png__decoder>,
        std::unique_ptr<wuffs_qoi__decoder>,
        std::unique_ptr<wuffs_targa__decoder>,
        std::unique_ptr<wuffs_wbmp__decoder>,
        std::unique_ptr<wuffs_webp__decoder>
    >;

    inline static constexpr auto init_dec(pathstr file, i32 fourcc, variantdec& fmtdec, wuffs_base__status& status, wuffs_base__image_decoder*& dec) {
        #define casedec(name, lname) case WUFFS_BASE__FOURCC__##name: { \
            auto& adec = fmtdec.emplace<std::unique_ptr<wuffs_##lname##__decoder>>(std::make_unique<wuffs_##lname##__decoder>()); \
            status = wuffs_##lname##__decoder__initialize(adec.get(), sizeof__wuffs_##lname##__decoder(), WUFFS_VERSION, WUFFS_INITIALIZE__DEFAULT_OPTIONS); \
            dec = wuffs_##lname##__decoder__upcast_as__wuffs_base__image_decoder(adec.get()); \
            break; \
        }

        switch (fourcc) {
            casedec(BMP,  bmp)
            casedec(GIF,  gif)
            casedec(JPEG, jpeg)
            casedec(NIE,  nie)
            casedec(NPBM, netpbm)
            casedec(PNG,  png)
            casedec(QOI,  qoi)
            casedec(TGA,  targa)
            casedec(WBMP, wbmp)
            casedec(WEBP, webp)
            default: log::uerror("unknown fourcc {} for the file {}", fourcc, file.cstr());
        }

        #undef casedec
    }

    inline static auto decode(const path& image) -> pack<vec<u8>, u32, u32> {
        pathstr file = image.str();
        auto raws = image.read_bytes();
        log::uassert(!raws.empty(), "the data of the image file {} is empty", file.cstr());

        wuffs_base__slice_u8 raw_slice = wuffs_base__make_slice_u8(raws.data(), raws.size());
        i32 fourcc = wuffs_base__magic_number_guess_fourcc(raw_slice, true);

        variantdec initaldec;
        wuffs_base__image_decoder* decoder = nullptr;
        wuffs_base__status status;
        init_dec(file, fourcc, initaldec, status, decoder);
        log::uassert(!wuffs_base__status__is_error(&status), "initialize decoder failed, wuffs error message: {}", wuffs_base__status__message(&status));

        wuffs_base__io_buffer src_buffer;
        src_buffer.data = wuffs_base__make_slice_u8(raws.data(), raws.size());
        src_buffer.meta.wi = raws.size();
        src_buffer.meta.ri = 0;
        src_buffer.meta.pos = 0;
        src_buffer.meta.closed = true;

        wuffs_base__image_config image_config;
        status = decoder->decode_image_config(&image_config, &src_buffer);
        log::uassert(!wuffs_base__status__is_error(&status), "get image config failed, wuffs error message: {}", wuffs_base__status__message(&status));


        wuffs_base__pixel_config pixel_config;
        wuffs_base__pixel_buffer pixel_buffer;
        uint32_t w = wuffs_base__pixel_config__width(&image_config.pixcfg);
        uint32_t h = wuffs_base__pixel_config__height(&image_config.pixcfg);
        wuffs_base__pixel_config__set(&pixel_config, WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL, WUFFS_BASE__PIXEL_SUBSAMPLING__NONE, w, h);

        size_t stride = 4 * w;
        size_t buffer_size = stride * h;
        vec<u8> pixels(buffer_size);
        wuffs_base__slice_u8 slice = wuffs_base__make_slice_u8(pixels.data(), buffer_size);
        status = wuffs_base__pixel_buffer__set_from_slice(&pixel_buffer, &pixel_config, slice);
        log::uassert(!wuffs_base__status__is_error(&status), "set pixel buffer failed, wuffs error message: {}", wuffs_base__status__message(&status));

        wuffs_base__range_ii_u64 workbuffer_len = decoder->workbuf_len();
        vec<u8> work_memory(workbuffer_len.max_incl, 0);
        wuffs_base__slice_u8 workbuf = wuffs_base__make_slice_u8(work_memory.data(), work_memory.size());

        status = decoder->decode_frame(&pixel_buffer, &src_buffer, WUFFS_BASE__PIXEL_BLEND__SRC, workbuf, nullptr);
        log::uassert(!wuffs_base__status__is_error(&status), "decode frame failed, wuffs error message: {}", wuffs_base__status__message(&status));

        return { std::move(pixels), w, h };
    }
}
