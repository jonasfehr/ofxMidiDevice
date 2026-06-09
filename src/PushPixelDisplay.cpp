#include "PushPixelDisplay.h"
#include "ofMain.h"
#include <cctype>
#include <cmath>

// ── Minimal 5×7 glyph bitmaps (column-major, bit 0 = top row) ────────────────

static const uint8_t GLYPH_SPACE[5] = {0,0,0,0,0};
static const uint8_t GLYPH_DASH[5]  = {0x00,0x08,0x08,0x08,0x00};
static const uint8_t GLYPH_DOT[5]   = {0x00,0x00,0x00,0x18,0x18};
static const uint8_t GLYPH_0[5]={0x3E,0x45,0x49,0x51,0x3E};
static const uint8_t GLYPH_1[5]={0x00,0x21,0x7F,0x01,0x00};
static const uint8_t GLYPH_2[5]={0x23,0x45,0x49,0x51,0x21};
static const uint8_t GLYPH_3[5]={0x22,0x41,0x49,0x49,0x36};
static const uint8_t GLYPH_4[5]={0x0C,0x14,0x24,0x7F,0x04};
static const uint8_t GLYPH_5[5]={0x72,0x51,0x51,0x51,0x4E};
static const uint8_t GLYPH_6[5]={0x3E,0x49,0x49,0x49,0x32};
static const uint8_t GLYPH_7[5]={0x40,0x47,0x48,0x50,0x60};
static const uint8_t GLYPH_8[5]={0x36,0x49,0x49,0x49,0x36};
static const uint8_t GLYPH_9[5]={0x26,0x49,0x49,0x49,0x3E};
static const uint8_t GLYPH_A[5]={0x3F,0x48,0x48,0x48,0x3F};
static const uint8_t GLYPH_B[5]={0x7F,0x49,0x49,0x49,0x36};
static const uint8_t GLYPH_C[5]={0x3E,0x41,0x41,0x41,0x22};
static const uint8_t GLYPH_D[5]={0x7F,0x41,0x41,0x22,0x1C};
static const uint8_t GLYPH_E[5]={0x7F,0x49,0x49,0x49,0x41};
static const uint8_t GLYPH_F[5]={0x7F,0x48,0x48,0x48,0x40};
static const uint8_t GLYPH_G[5]={0x3E,0x41,0x49,0x49,0x2E};
static const uint8_t GLYPH_H[5]={0x7F,0x08,0x08,0x08,0x7F};
static const uint8_t GLYPH_I[5]={0x41,0x41,0x7F,0x41,0x41};
static const uint8_t GLYPH_J[5]={0x02,0x01,0x41,0x7E,0x40};
static const uint8_t GLYPH_K[5]={0x7F,0x08,0x14,0x22,0x41};
static const uint8_t GLYPH_L[5]={0x7F,0x01,0x01,0x01,0x01};
static const uint8_t GLYPH_M[5]={0x7F,0x20,0x10,0x20,0x7F};
static const uint8_t GLYPH_N[5]={0x7F,0x10,0x08,0x04,0x7F};
static const uint8_t GLYPH_O[5]={0x3E,0x41,0x41,0x41,0x3E};
static const uint8_t GLYPH_P[5]={0x7F,0x48,0x48,0x48,0x30};
static const uint8_t GLYPH_Q[5]={0x3E,0x41,0x45,0x42,0x3D};
static const uint8_t GLYPH_R[5]={0x7F,0x48,0x4C,0x4A,0x31};
static const uint8_t GLYPH_S[5]={0x32,0x49,0x49,0x49,0x26};
static const uint8_t GLYPH_T[5]={0x40,0x40,0x7F,0x40,0x40};
static const uint8_t GLYPH_U[5]={0x7E,0x01,0x01,0x01,0x7E};
static const uint8_t GLYPH_V[5]={0x7C,0x02,0x01,0x02,0x7C};
static const uint8_t GLYPH_W[5]={0x7E,0x01,0x06,0x01,0x7E};
static const uint8_t GLYPH_X[5]={0x63,0x14,0x08,0x14,0x63};
static const uint8_t GLYPH_Y[5]={0x60,0x10,0x0F,0x10,0x60};
static const uint8_t GLYPH_Z[5]={0x43,0x45,0x49,0x51,0x61};

static inline const uint8_t* glyphFor(char c) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    switch (c) {
        case ' ': return GLYPH_SPACE;
        case '-': return GLYPH_DASH;
        case '.': return GLYPH_DOT;
        case '0': return GLYPH_0; case '1': return GLYPH_1; case '2': return GLYPH_2;
        case '3': return GLYPH_3; case '4': return GLYPH_4; case '5': return GLYPH_5;
        case '6': return GLYPH_6; case '7': return GLYPH_7; case '8': return GLYPH_8;
        case '9': return GLYPH_9;
        case 'A': return GLYPH_A; case 'B': return GLYPH_B; case 'C': return GLYPH_C;
        case 'D': return GLYPH_D; case 'E': return GLYPH_E; case 'F': return GLYPH_F;
        case 'G': return GLYPH_G; case 'H': return GLYPH_H; case 'I': return GLYPH_I;
        case 'J': return GLYPH_J; case 'K': return GLYPH_K; case 'L': return GLYPH_L;
        case 'M': return GLYPH_M; case 'N': return GLYPH_N; case 'O': return GLYPH_O;
        case 'P': return GLYPH_P; case 'Q': return GLYPH_Q; case 'R': return GLYPH_R;
        case 'S': return GLYPH_S; case 'T': return GLYPH_T; case 'U': return GLYPH_U;
        case 'V': return GLYPH_V; case 'W': return GLYPH_W; case 'X': return GLYPH_X;
        case 'Y': return GLYPH_Y; case 'Z': return GLYPH_Z;
        default:  return GLYPH_SPACE;
    }
}

// ── PushPixelDisplay ──────────────────────────────────────────────────────────

PushPixelDisplay::PushPixelDisplay(uint16_t vid, uint16_t pid,
                                    uint8_t iface, uint8_t alt, uint8_t ep,
                                    uint64_t keepAliveMs)
    : vid_(vid), pid_(pid), iface_(iface), alt_(alt), ep_(ep)
    , keepAliveMs_(keepAliveMs) {}

PushPixelDisplay::~PushPixelDisplay() {
    close();
}

bool PushPixelDisplay::open() {
    if (!transport_)
        transport_ = std::make_unique<PushDisplayTransport>(vid_, pid_, iface_, alt_, ep_);
    if (!transport_->isOpen()) {
        if (!transport_->open()) {
            ofLogError("PushPixelDisplay") << "USB display transport open failed";
            return false;
        }
    }
    startThread();
    return true;
}

bool PushPixelDisplay::isOpen() const {
    return transport_ && transport_->isOpen();
}

void PushPixelDisplay::close() {
    stopThread();
    if (transport_) transport_->close();
}

void PushPixelDisplay::restart() {
    close();
    open();
}

// ── Thread lifecycle ──────────────────────────────────────────────────────────

void PushPixelDisplay::startThread() {
    stopThread();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_  = false;
        dirty_ = false;
    }
    thread_ = std::thread(&PushPixelDisplay::threadFunc, this);
}

void PushPixelDisplay::stopThread() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_one();
    if (thread_.joinable()) thread_.join();
}

void PushPixelDisplay::threadFunc() {
    while (true) {
        std::string              title;
        std::vector<std::string> labels;
        std::vector<float>       values;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return dirty_ || stop_; });
            if (stop_) break;
            title  = title_;
            labels = labels_;
            values = values_;
            dirty_ = false;
        }
        if (!title.empty()) {
            renderAndSend(title, labels, values);
            lastSentMs_ = ofGetElapsedTimeMillis();
        }
    }
}

void PushPixelDisplay::postFrame() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        dirty_ = true;
    }
    cv_.notify_one();
}

// ── Public interface ──────────────────────────────────────────────────────────

void PushPixelDisplay::showPage(const std::string& title) {
    bool notify = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (title.empty()) {
            // Empty title means re-post current state (page-transition keepalive).
            if (!title_.empty()) { dirty_ = true; notify = true; }
        } else if (title != title_) {
            title_ = title;
            dirty_ = true;
            notify = true;
        }
    }
    if (notify) cv_.notify_one();
}

void PushPixelDisplay::showParameters(const std::vector<std::string>& labels,
                                       const std::vector<float>& values) {
    bool changed = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (labels != labels_) { labels_ = labels; changed = true; }
        if (!changed) {
            if (values.size() != values_.size()) {
                changed = true;
            } else {
                for (size_t i = 0; i < values.size(); ++i) {
                    if (std::abs(values[i] - values_[i]) > 0.01f) { changed = true; break; }
                }
            }
        }
        if (changed) { values_ = values; dirty_ = true; }
    }
    if (changed) cv_.notify_one();
}

void PushPixelDisplay::tick(uint64_t nowMs) {
    if (!isOpen() || keepAliveMs_ == 0) return;
    if (nowMs - lastSentMs_.load() < keepAliveMs_) return;
    bool notify = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!title_.empty()) { dirty_ = true; notify = true; }
    }
    if (notify) cv_.notify_one();
}

// ── Glyph rendering ───────────────────────────────────────────────────────────

void PushPixelDisplay::blitChar(std::vector<uint16_t>& buf,
                                 int x, int y, char c, uint16_t color) {
    const uint8_t* g = glyphFor(c);
    for (int col = 0; col < 5; ++col) {
        uint8_t bits = g[col];
        for (int row = 0; row < 7; ++row) {
            if (bits & (1 << row)) {
                int px = x + col;
                int py = y + (6 - row);
                if (px >= 0 && px < kWidth && py >= 0 && py < kHeight)
                    buf[py * kWidth + px] = color;
            }
        }
    }
}

void PushPixelDisplay::blitCharScaled(std::vector<uint16_t>& buf,
                                       int x, int y, char c, uint16_t color, int scale) {
    const uint8_t* g = glyphFor(c);
    for (int col = 0; col < 5; ++col) {
        uint8_t bits = g[col];
        for (int row = 0; row < 7; ++row) {
            if (bits & (1 << row)) {
                for (int dy = 0; dy < scale; ++dy) {
                    for (int dx = 0; dx < scale; ++dx) {
                        int px = x + col * scale + dx;
                        int py = y + (6 - row) * scale + dy;
                        if (px >= 0 && px < kWidth && py >= 0 && py < kHeight)
                            buf[py * kWidth + px] = color;
                    }
                }
            }
        }
    }
}

// ── Frame render + USB send ───────────────────────────────────────────────────

void PushPixelDisplay::renderAndSend(const std::string& title,
                                      const std::vector<std::string>& labels,
                                      const std::vector<float>& values) {
    if (!transport_ || !transport_->isOpen()) return;

    std::vector<uint16_t> pixels(kWidth * kHeight, 0x0000);

    auto drawText = [&](int x, int y, const std::string& text) {
        int cursor = x;
        for (char c : text) { blitChar(pixels, cursor, y, c, 0xFFFF); cursor += 6; }
    };
    auto drawTextScaled = [&](int x, int y, const std::string& text, int scale) {
        int cursor = x;
        for (char c : text) { blitCharScaled(pixels, cursor, y, c, 0xFFFF, scale); cursor += 6 * scale; }
    };

    if (!title.empty())
        drawTextScaled(8, kTitleY, title.substr(0, 30), 2);

    const int cols  = 8;
    const int slotW = kWidth / cols;
    for (size_t i = 0; i < labels.size() && i < 8; ++i) {
        int col  = static_cast<int>(i % cols);
        int x    = col * slotW + 4;
        int y    = kRow1Y + kTitleY;
        drawText(x, y, labels[i].substr(0, 12));

        float v    = (i < values.size()) ? ofClamp(values[i], 0.0f, 1.0f) : 0.0f;
        int barX   = col * slotW + 4;
        int barY   = y + 10;
        int barW   = slotW - 8;
        int barH   = 6;
        int fillW  = static_cast<int>(barW * v);
        uint16_t frameColor = 0x7BEF;
        for (int yy = 0; yy < barH; ++yy) {
            for (int xx = 0; xx < barW; ++xx) {
                pixels[(barY + yy) * kWidth + (barX + xx)] =
                    (xx < fillW) ? 0xFFFF : 0x0000;
            }
        }
        for (int xx = 0; xx < barW; ++xx) {
            pixels[barY             * kWidth + (barX + xx)] = frameColor;
            pixels[(barY + barH - 1)* kWidth + (barX + xx)] = frameColor;
        }
        for (int yy = 0; yy < barH; ++yy) {
            pixels[(barY + yy) * kWidth + barX]            = frameColor;
            pixels[(barY + yy) * kWidth + (barX + barW-1)] = frameColor;
        }
    }

    // Push3 display header
    const uint8_t header[16] = {0xFF,0xCC,0xAA,0x88,0,0,0,0,0,0,0,0,0,0,0,0};
    if (!transport_->sendBuffer(header, sizeof(header))) {
        ofLogError("PushPixelDisplay") << "header transfer failed";
        return;
    }

    // Per-row payload: 1920 bytes pixel data + 128 padding, XOR-masked
    static const uint8_t xorMask[4] = {0xE7, 0xF3, 0xE7, 0xFF};
    const int rowBytes = kWidth * 2;
    std::vector<uint8_t> rowBuf(kStrideBytes, 0);
    for (int y = 0; y < kHeight; ++y) {
        std::fill(rowBuf.begin(), rowBuf.end(), 0);
        const uint16_t* row = &pixels[y * kWidth];
        uint8_t* dst = rowBuf.data();
        for (int x = 0; x < kWidth; ++x) {
            uint16_t p = row[x];
            dst[2*x]     = static_cast<uint8_t>(p & 0xFF);
            dst[2*x + 1] = static_cast<uint8_t>((p >> 8) & 0xFF);
        }
        for (int i = 0; i < rowBytes; ++i) dst[i] ^= xorMask[i & 0x03];
        if (!transport_->sendBuffer(rowBuf.data(), kStrideBytes)) {
            ofLogError("PushPixelDisplay") << "row transfer failed at y=" << y;
            return;
        }
    }
}
