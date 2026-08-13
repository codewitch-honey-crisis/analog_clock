#pragma once
#include <time.h>
#include <gfx.hpp>
#include <uix.hpp>

// colors for the UI
using color_t = gfx::color<gfx::rgb_pixel<16>>; // native
using color32_t = gfx::color<gfx::rgba_pixel<32>>; // uix

// the screen template instantiation aliases
using screen_t = uix::screen<gfx::rgb_pixel<16>>;
using surface_t = screen_t::control_surface_type;

template<typename ControlSurfaceType>
class aa_clock : public uix::control<ControlSurfaceType> {
    using base_type = uix::control<ControlSurfaceType>;
public:
    using type = aa_clock;
    using control_surface_type = ControlSurfaceType;
private:
    constexpr static const  int16_t default_face_border_width = 2;
    constexpr static const  typename ControlSurfaceType::pixel_type default_face_border_color = gfx::color<typename ControlSurfaceType::pixel_type>::black;
    constexpr static const  typename ControlSurfaceType::pixel_type default_face_color = gfx::color<typename ControlSurfaceType::pixel_type>::white;
    constexpr static const  typename ControlSurfaceType::pixel_type default_tick_border_color = gfx::color<typename ControlSurfaceType::pixel_type>::gray;
    constexpr static const  typename ControlSurfaceType::pixel_type default_tick_color = gfx::color<typename ControlSurfaceType::pixel_type>::gray;
    constexpr static const  int16_t default_tick_border_width = 2;
    constexpr static const  typename ControlSurfaceType::pixel_type default_minute_color = gfx::color<typename ControlSurfaceType::pixel_type>::black;
    constexpr static const  typename ControlSurfaceType::pixel_type default_minute_border_color = gfx::color<typename ControlSurfaceType::pixel_type>::gray;
    constexpr static const  int16_t default_minute_border_width = 2;
    constexpr static const  typename ControlSurfaceType::pixel_type default_hour_color = gfx::color<typename ControlSurfaceType::pixel_type>::black;
    constexpr static const  typename ControlSurfaceType::pixel_type default_hour_border_color = gfx::color<typename ControlSurfaceType::pixel_type>::gray;
    constexpr static const  int16_t default_hour_border_width = 2;
    constexpr static const  typename ControlSurfaceType::pixel_type default_second_color = gfx::color<typename ControlSurfaceType::pixel_type>::red;
    constexpr static const  typename ControlSurfaceType::pixel_type default_second_border_color = gfx::color<typename ControlSurfaceType::pixel_type>::red;
    constexpr static const  int16_t default_second_border_width = 2;
    using fb_t = gfx::bitmap<typename control_surface_type::pixel_type,typename control_surface_type::palette_type>;
    int16_t m_face_border_width;
    typename ControlSurfaceType::pixel_type m_face_border_color;
    typename ControlSurfaceType::pixel_type m_face_color;
    typename ControlSurfaceType::pixel_type m_tick_border_color;
    typename ControlSurfaceType::pixel_type m_tick_color;
    int16_t m_tick_border_width;
    typename ControlSurfaceType::pixel_type m_minute_color;
    typename ControlSurfaceType::pixel_type m_minute_border_color;
    int16_t m_minute_border_width;
    typename ControlSurfaceType::pixel_type m_hour_color;
    typename ControlSurfaceType::pixel_type m_hour_border_color;
    int16_t m_hour_border_width;
    typename ControlSurfaceType::pixel_type m_second_color;
    typename ControlSurfaceType::pixel_type m_second_border_color;
    int16_t m_second_border_width;
    time_t m_time;
    bool m_face_dirty;
    bool m_buffer_face;
    fb_t m_face_buffer;
    gfx::mask_draw_cache* m_dc;
    gfx::mask_draw_cache m_local_dc;
    // compute thetas for a rotation
    static void update_transform(float rotation, float& ctheta, float& stheta) {
        float rads = gfx::math::deg2rad(rotation); // rotation * (3.1415926536f / 180.0f);
        ctheta = cosf(rads);
        stheta = sinf(rads);
    }
    // transform a point given some thetas, a center and an offset
    static gfx::spoint16 transform_point(float ctheta, float stheta, gfx::spoint16 center, gfx::spoint16 offset, int16_t x, int16_t y) {
        float rx = (ctheta * (x - (float)center.x) - stheta * (y - (float)center.y) + (float)center.x) + offset.x;
        float ry = (stheta * (x - (float)center.x) + ctheta * (y - (float)center.y) + (float)center.y) + offset.y;
        return {(int16_t)roundf(rx), (int16_t)roundf(ry)};
    }
    template<typename Destination>
    gfx::gfx_result draw_clock_face(Destination& destination) {   
        constexpr static const int rot_step = 360 / 12;
        gfx::spoint16 offset(0, 0);
        gfx::spoint16 center(0, 0);
        gfx::mask_draw_cache* dc = (m_dc!=nullptr)?m_dc:&m_local_dc;
        float rotation(0);
        float ctheta, stheta;
        gfx::ssize16 size = (gfx::ssize16)destination.dimensions();
        gfx::srect16 b = size.bounds();
        b.inflate_inplace(-m_face_border_width - 1, -m_face_border_width - 1);
        int16_t w = b.width();
        int16_t h = b.height();
        if(w>h) w= h;
        gfx::srect16 face_sq(b.x1, b.y1, b.x1 + w - 1, b.y1 + w - 1);
        gfx::srect16 sr(0, w / 30, w / 30, w / 5);
        sr.center_horizontal_inplace(face_sq);
        center = gfx::spoint16(w * 0.5f + m_face_border_width + 1, w * 0.5f + m_face_border_width + 1);
        gfx::gfx_result res=gfx::draw::aa_filled_arc(destination,gfx::srect16(center,center.x-1),m_face_color,0,360,dc);
        if(res!=gfx::gfx_result::success) {
            return res;
        }
        res=gfx::draw::aa_arc(destination,gfx::srect16(center,center.x-1),m_face_border_color,0,360,m_face_border_width,gfx::line_cap::butt,dc);
        if(res!=gfx::gfx_result::success) {
            return res;
        }
        bool toggle = false;
        gfx::spoint16 points[4];
        for (float rot = 0; rot < 360.0f; rot += rot_step) {
            rotation = rot;
            update_transform(rotation, ctheta, stheta);
            toggle = !toggle;
            if (toggle) {
                points[0]=transform_point(ctheta, stheta, center, offset, sr.x1, sr.y1);
                points[1]=transform_point(ctheta, stheta, center, offset, sr.x2, sr.y1);
                points[2]=transform_point(ctheta, stheta, center, offset, sr.x2, sr.y2);
                points[3]=transform_point(ctheta, stheta, center, offset, sr.x1, sr.y2);
            } else {
                points[0]=transform_point(ctheta, stheta, center, offset, sr.x1, sr.y1);
                points[1]=transform_point(ctheta, stheta, center, offset, sr.x2, sr.y1);
                points[2]=transform_point(ctheta, stheta, center, offset, sr.x2, sr.y2 - sr.height() * 0.5f);
                points[3]=transform_point(ctheta, stheta, center, offset, sr.x1, sr.y2 - sr.height() * 0.5f);
            }
            gfx::spath16 path(4,points);
            res = gfx::draw::aa_filled_polygon(destination,path,m_tick_color,gfx::fill_rule::even_odd,dc);
            if(res!=gfx::gfx_result::success) {
                return res;
            }
            res = gfx::draw::aa_polygon(destination,path,m_tick_border_color,m_tick_border_width,gfx::line_join::miter,4, dc);
            if(res!=gfx::gfx_result::success) {
                return res;
            }
        }    
        return gfx::gfx_result::success;
    }
    void draw_clock_time(control_surface_type& destination) {
        gfx::mask_draw_cache* dc = (m_dc!=nullptr)?m_dc:&m_local_dc;
        gfx::spoint16 offset(0, 0);
        gfx::spoint16 center(0, 0);
        float rotation(0);
        float ctheta, stheta;
        time_t time = m_time;
        gfx::ssize16 size = (gfx::ssize16)destination.dimensions();
        gfx::srect16 b = gfx::ssize16(size.width, size.height).bounds();
        b.inflate_inplace(-m_face_border_width - 1, -m_face_border_width - 1);
        float w = b.width();
        float h = b.height();
        if(w>h) w= h;
        if(h>w) h= w;
        center = gfx::spoint16(w * 0.5f + m_face_border_width + 1, w * 0.5f + m_face_border_width + 1);
        gfx::srect16 face_sq(b.x1, b.y1, b.x1 + (int)w - 1, b.y1 + (int)w - 1);
        gfx::srect16 sr = gfx::srect16(0, w / 40, w / 16, w / 2);
        sr.center_horizontal_inplace(face_sq);
        // create a path for the minute hand:
        rotation = (fmodf(time / 60.0f, 60) / 60.0f) * 360.0f;
        update_transform(rotation, ctheta, stheta);
        gfx::spoint16 points[4];
        points[0]=transform_point(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y1);
        points[1]=transform_point(ctheta, stheta, center, offset, sr.x2, sr.y2);
        points[2]=transform_point(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y2 + (w / 20));
        points[3]=transform_point(ctheta, stheta, center, offset, sr.x1, sr.y2);
        gfx::spath16 path(4,points);
        gfx::draw::aa_filled_polygon(destination,path,m_minute_color,gfx::fill_rule::even_odd,dc);
        gfx::draw::aa_polygon(destination,path,m_minute_color,m_minute_border_width,gfx::line_join::miter,4,dc);
        // create a path for the hour hand
        sr.y1 += w / 8;
        rotation = (fmodf(time / (3600.0f), 12.0f) / (12.0f)) * 360.0f;
        update_transform(rotation, ctheta, stheta);
        points[0]=transform_point(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y1);
        points[1]=transform_point(ctheta, stheta, center, offset, sr.x2, sr.y2);
        points[2]=transform_point(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y2 + (w / 20));
        points[3]=transform_point(ctheta, stheta, center, offset, sr.x1, sr.y2);
        gfx::draw::aa_filled_polygon(destination,path,m_hour_color,gfx::fill_rule::even_odd,dc);
        gfx::draw::aa_polygon(destination,path,m_hour_color,m_hour_border_width,gfx::line_join::miter,4,dc);
        // create a path for the second hand
        sr.y1 -= w / 8;
        rotation = ((time % 60) / 60.0f) * 360.0f;
        update_transform(rotation, ctheta, stheta);
        points[0]=transform_point(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y1);
        points[1]=transform_point(ctheta, stheta, center, offset, sr.x2, sr.y2);
        points[2]=transform_point(ctheta, stheta, center, offset, sr.x1 + sr.width() * 0.5f, sr.y2 + (w / 20));
        points[3]=transform_point(ctheta, stheta, center, offset, sr.x1, sr.y2);
        gfx::draw::aa_filled_polygon(destination,path,m_second_color,gfx::fill_rule::even_odd,dc);
        gfx::draw::aa_polygon(destination,path,m_second_color,m_second_border_width,gfx::line_join::miter,4,dc);
    }
public:
    aa_clock() : base_type(),
            m_face_border_width(default_face_border_width),
            m_face_border_color(default_face_border_color),
            m_face_color(default_face_color),
            m_tick_color(default_tick_color),
            m_tick_border_width(default_tick_border_width),
            m_minute_color(default_minute_color),
            m_minute_border_color(default_minute_border_color),
            m_minute_border_width(default_minute_border_width),
            m_hour_color(default_hour_color),
            m_hour_border_color(default_hour_border_color),
            m_hour_border_width(default_hour_border_width),
            m_second_color(default_second_color),
            m_second_border_color(default_second_border_color),
            m_second_border_width(default_second_border_width),
            m_time(0),
            m_face_dirty(true),
            m_buffer_face(true),
            m_dc(nullptr)
              {

    }
    aa_clock(const aa_clock& rhs) {
        *this = rhs;
    }
    aa_clock(aa_clock&& rhs) {
        *this = rhs;
    }
    aa_clock& operator=(const aa_clock& rhs) {
        *this = rhs;
    }
    aa_clock& operator=(aa_clock&& rhs) {
        *this=rhs;
    }
    virtual ~aa_clock() {
        if(m_face_buffer.begin()) {
            free(m_face_buffer.begin());
        }
    }
protected:
    
    virtual void on_paint(control_surface_type& destination, const uix::srect16& clip) override {
        if(m_buffer_face && m_face_dirty) {
            typename control_surface_type::pixel_type bg;
            destination.point({0,0},&bg);
            int16_t w = this->dimensions().width;
            int16_t h = this->dimensions().height;
            if(w>h) w= h;
            fb_t bmp = gfx::create_bitmap<typename fb_t::pixel_type,typename fb_t::palette_type>(gfx::size16(w,w));
            if(bmp.begin()) {
                bmp.fill(bmp.bounds(),bg);
                if(gfx::gfx_result::success==draw_clock_face(bmp)) {
                    if(m_face_buffer.begin()!=nullptr) {
                        free(m_face_buffer.begin());
                    }
                    m_face_buffer = bmp;
                    m_face_dirty = false;
                }
            }
        }
        if(m_buffer_face && !m_face_dirty) {
            // we have a valid bitmap, no need to draw the face here.
            gfx::draw::bitmap(destination,m_face_buffer.bounds(),m_face_buffer,m_face_buffer.bounds());
        }
        if(!m_buffer_face || m_face_dirty) {
            draw_clock_face(destination);
        }
        draw_clock_time(destination);
    }
public:
    uint16_t face_border_width() const {
        return m_face_border_width;
    }
    void face_border_width(int16_t value) {
        m_face_border_width = value;
        m_face_dirty = true;
        this->invalidate();
    }
    gfx::rgba_pixel<32> face_border_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_face_border_color,&result);
        return result;
    }
    void face_border_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_face_border_color);
        m_face_dirty = true;
        this->invalidate();
    }
    gfx::rgba_pixel<32> face_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_face_color,&result);
        return result;
    }
    void face_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_face_color);
        m_face_dirty = true;
        this->invalidate();
    }
    gfx::rgba_pixel<32> tick_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_tick_color,&result);
        return result;
    }
    void tick_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_tick_color);
        m_face_dirty = true;
        this->invalidate();
    }
    int16_t tick_border_width() const {
        return m_tick_border_width;
    }
    void tick_border_width(int16_t value) {
        m_tick_border_width = value;
        m_face_dirty = true;
        this->invalidate();
    }
    gfx::rgba_pixel<32> minute_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_minute_color,&result);
        return result;
    }
    void minute_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_minute_color);
        this->invalidate();
    }
    gfx::rgba_pixel<32> minute_border_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_minute_border_color,&result);
        return result;
    }
    void minute_border_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_minute_border_color);
        this->invalidate();
    }
    int16_t minute_border_width() const {
        return m_minute_border_width;
    }
    void minute_border_width(int16_t value) {
        m_minute_border_width = value;
        this->invalidate();
    }
    gfx::rgba_pixel<32> hour_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_hour_color,&result);
        return result;
    }
    void hour_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_hour_color);
        this->invalidate();
    }
    gfx::rgba_pixel<32> hour_border_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_hour_border_color,&result);
        return result;
    }
    void hour_border_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_hour_border_color);
        this->invalidate();
    }
    int16_t hour_border_width() const {
        return m_hour_border_width;
    }
    void hour_border_width(int16_t value) {
        m_hour_border_width = value;
        this->invalidate();
    }
    gfx::rgba_pixel<32> second_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_second_color,&result);
        return result;
    }
    void second_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_second_color);
        this->invalidate();
    }
    gfx::rgba_pixel<32> second_border_color() const {
        gfx::rgba_pixel<32> result;
        convert(m_second_border_color,&result);
        return result;
    }
    void second_border_color(gfx::rgba_pixel<32> value) {
        convert(value,&m_second_border_color);
        this->invalidate();
    }
    int16_t second_border_width() const {
        return m_second_border_width;
    }
    void second_border_width(int16_t value) {
        m_second_border_width = value;
        this->invalidate();
    }
    time_t time() const {
        return m_time;
    }
    void time(time_t value) {
        m_time = value;
        this->invalidate();
    }
    bool buffer_face() const {
        return m_buffer_face;
    }
    void buffer_face(bool value) {
        if(value==false) {
            if(m_buffer_face) {
                if(m_face_buffer.begin()) {
                    free(m_face_buffer.begin());
                }
            }
        }
        m_buffer_face = value;
    }
    gfx::mask_draw_cache* draw_cache() const {
        return m_dc;
    }
    void draw_cache(gfx::mask_draw_cache* cache) {
        m_dc = cache;
    }
};
