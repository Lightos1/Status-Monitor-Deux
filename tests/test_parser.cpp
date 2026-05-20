// test_parser.cpp  –  Host-side sanitizer test driver for smd::Document.
//
// Compiles alongside smd_parser.cpp + tinyexpr.c (as C) without DevkitPro.
// Exercises LoadFromFile/Memory, Compile, Evaluate, Reset, Free, and every
// public config accessor across all .smd fixtures, plus edge-case inputs.
//
// Exit code: 0 on success, non-zero if any test fails.

#include "smd_parser.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Test infrastructure
// ---------------------------------------------------------------------------

static int g_pass = 0;
static int g_fail = 0;

#define EXPECT(cond, msg) \
    do { \
        if (cond) { \
            ++g_pass; \
        } else { \
            ++g_fail; \
            std::fprintf(stderr, "FAIL [%s:%d] %s\n", __FILE__, __LINE__, (msg)); \
        } \
    } while (0)

// Render callback – non-const ref to match the Callback typedef exactly.
struct RenderCtx {
    int calls = 0;
    int text  = 0;
    int box   = 0;
    int graph = 0;
};

static void render_cb(smd::RenderCommand& cmd, void* user) {
    auto* ctx = static_cast<RenderCtx*>(user);
    ++ctx->calls;
    switch (cmd.type) {
        case smd::RenderCmdType::Text:           ++ctx->text;  break;
        case smd::RenderCmdType::Box:
        case smd::RenderCmdType::RoundedBox:
        case smd::RenderCmdType::EmptyBox:       ++ctx->box;   break;
        case smd::RenderCmdType::GraphLineChart:  ++ctx->graph; break;
        case smd::RenderCmdType::GetDimensions:
            // Provide non-zero dims so dependent expressions don't divide-by-zero.
            if (cmd.outDims) { cmd.outDims->x = 100; cmd.outDims->y = 20; }
            break;
        default: break;
    }
}

// ---------------------------------------------------------------------------
// Helper: load a file into memory
// ---------------------------------------------------------------------------
static std::string slurp(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// ---------------------------------------------------------------------------
// Helper: bind the standard runtime variables referenced by shipped .smd files
// ---------------------------------------------------------------------------
static void bind_live_vars(smd::Document& doc,
                           const int64_t& cpu_hz, const int64_t& gpu_hz,
                           const int64_t& ram_hz, const int64_t& fps,
                           const int64_t& bat_pct,
                           const float& cpu_f, const double& temp_d,
                           const bool& show_fps, const bool& show_real,
                           const bool& salty_running, const bool& clk_running,
                           const std::string& game_name,
                           const smd::ResolutionEntry* res_arr) {
    doc.BindInt64 ("CPU_Hz_int",                       &cpu_hz);
    doc.BindInt64 ("GPU_Hz_int",                       &gpu_hz);
    doc.BindInt64 ("RAM_Hz_int",                       &ram_hz);
    doc.BindInt64 ("FPS_int",                          &fps);
    doc.BindInt64 ("BAT_Percent_int",                  &bat_pct);
    doc.BindFloat ("CPU_Hz_float",                     &cpu_f);
    doc.BindDouble("Temp_CPU_double",                  &temp_d);
    doc.BindBool  ("SaltyNX_Running",                  &salty_running);
    doc.BindBool  ("SysClk_Running",                   &clk_running);
    doc.BindBool  ("User_ShowRealFrequencies",          &show_real);
    doc.BindBool  ("User_ShowFPS",                     &show_fps);
    doc.BindString("Game_Name",                        &game_name);
    doc.BindResolutionArray("Game_ResolutionViewportCalls_int", res_arr);
    doc.BindResolutionArray("Game_ResolutionRenderCalls_int",   res_arr);
}

// ---------------------------------------------------------------------------
// Test 1: load every .smd fixture from the modes/ directory
// ---------------------------------------------------------------------------
static void test_fixture_files(const fs::path& modes_dir) {
    if (!fs::exists(modes_dir)) {
        std::fprintf(stderr, "SKIP fixture tests: %s not found\n",
                     modes_dir.c_str());
        return;
    }

    // Live bind values – realistic non-zero defaults.
    const int64_t cpu_hz = 1785000000LL;
    const int64_t gpu_hz = 768000000LL;
    const int64_t ram_hz = 1600000000LL;
    const int64_t fps    = 60;
    const int64_t bat    = 80;
    const float   cpu_f  = 1785.0f;
    const double  temp   = 45.0;
    const bool    show_fps    = true;
    const bool    show_real   = true;
    const bool    salty = true;
    const bool    clk   = true;
    const std::string game = "TestGame";

    smd::ResolutionEntry res_arr[8] = {};
    res_arr[0] = {1920, 1080, 5};
    res_arr[1] = {1280, 720,  3};

    int fixture_count = 0;

    for (auto& entry : fs::recursive_directory_iterator(modes_dir)) {
        if (!entry.is_regular_file()) continue;
        const auto ext = entry.path().extension().string();
        if (ext != ".smd" && ext != ".smf") continue;

        ++fixture_count;
        const std::string path_str = entry.path().string();
        std::fprintf(stdout, "  fixture: %s\n", path_str.c_str());

        // --- Load from file ---
        {
            smd::Document doc;
            bool loaded = doc.LoadFromFile(path_str.c_str());
            EXPECT(loaded, ("LoadFromFile failed: " + path_str).c_str());
            if (!loaded) continue;

            bind_live_vars(doc, cpu_hz, gpu_hz, ram_hz, fps, bat,
                           cpu_f, temp, show_fps, show_real, salty, clk,
                           game, res_arr);

            bool compiled = doc.Compile();
            EXPECT(compiled, ("Compile failed: " + path_str).c_str());
            if (!compiled) continue;

            RenderCtx ctx1;
            bool ev1 = doc.Evaluate(render_cb, &ctx1);
            EXPECT(ev1, ("Evaluate pass 1 failed: " + path_str).c_str());

            doc.Reset();

            RenderCtx ctx2;
            bool ev2 = doc.Evaluate(render_cb, &ctx2);
            EXPECT(ev2, ("Evaluate pass 2 (post-Reset) failed: " + path_str).c_str());

            // Config accessors must not crash.
            (void)doc.GetConfigInt   ("LayerWidth",  448);
            (void)doc.GetConfigInt   ("LayerHeight", 720);
            (void)doc.GetConfigBool  ("Movable",     false);
            (void)doc.GetConfigString("Name",        "");

            // FormatConfigString on a known key.
            std::string fmtOut;
            (void)doc.FormatConfigString("Name", fmtOut);
        }

        // --- Load from memory (same file content) ---
        {
            std::string content = slurp(entry.path());
            smd::Document doc;
            bool loaded = doc.LoadFromMemory(content.data(), content.size());
            EXPECT(loaded, ("LoadFromMemory failed: " + path_str).c_str());

            if (loaded) {
                doc.BindInt64("FPS_int", &fps);
                bool compiled = doc.Compile();
                EXPECT(compiled,
                       ("Compile (from memory) failed: " + path_str).c_str());
                if (compiled) {
                    RenderCtx ctx;
                    bool ev = doc.Evaluate(render_cb, &ctx);
                    EXPECT(ev, ("Evaluate (from memory) failed: " +
                                path_str).c_str());
                }
            }
        }
    }

    if (fixture_count == 0)
        std::fprintf(stderr,
                     "WARNING: no .smd fixtures found under %s\n",
                     modes_dir.c_str());
    else
        std::fprintf(stdout, "  loaded %d fixture(s)\n", fixture_count);
}

// ---------------------------------------------------------------------------
// Test 2: multi-load / re-use of the same Document object
// ---------------------------------------------------------------------------
static void test_reload(const fs::path& modes_dir) {
    fs::path first_smd;
    for (auto& e : fs::recursive_directory_iterator(modes_dir))
        if (e.is_regular_file() && e.path().extension() == ".smd")
            { first_smd = e.path(); break; }
    if (first_smd.empty()) return;

    const int64_t fps = 60;
    smd::Document doc;
    for (int i = 0; i < 3; ++i) {
        bool loaded = doc.LoadFromFile(first_smd.c_str());
        EXPECT(loaded, "reload: LoadFromFile");
        if (!loaded) continue;
        doc.BindInt64("FPS_int", &fps);
        bool compiled = doc.Compile();
        EXPECT(compiled, "reload: Compile");
        if (!compiled) continue;
        RenderCtx ctx;
        bool ev = doc.Evaluate(render_cb, &ctx);
        EXPECT(ev, "reload: Evaluate");
        doc.Free();
    }
}

// ---------------------------------------------------------------------------
// Test 3: malformed / edge-case inputs (must not crash, return gracefully)
// ---------------------------------------------------------------------------
static void test_malformed() {
    // Build the long-string case as a persistent std::string so c_str() stays valid.
    static const std::string kLongStr =
        "Name = X\nTEXT{\"" + std::string(4096, 'a') + "\",0,0,0,0xFFFF}\n";

    struct Case { const char* label; const char* src; size_t len; };
    const Case cases[] = {
        { "empty",              "",                          0 },
        { "whitespace only",    "   \n\t\r\n  ",             14 },
        { "comment only",       ";; just a comment\n",       19 },
        { "minimal name",       "Name = Test\n",             12 },
        { "unclosed brace",     "Name = X\nTEXT{\"hello\", 0, 0, 0, 0xFFFF\n", 39 },
        { "missing arg",        "Name = X\nTEXT{}\n",        17 },
        { "deep nesting",
          "Name = X\n#if 1\n#if 1\n#if 1\n#if 1\n"
          "TEXT{\"x\",0,0,0,0xFFFF}\n#end\n#end\n#end\n#end\n", 0 },
        { "for loop empty list",
          "Name = X\n#for $x in $nolist\nTEXT{\"x\",0,0,0,0xFFFF}\n#end\n", 0 },
        { "expr division zero", "Name = X\nTEXT{\"{1/0}\",0,0,0,0xFFFF}\n", 0 },
        { "long string",        kLongStr.c_str(),            kLongStr.size() },
        { "config only",
          "Name = Parser Test\nLayerWidth = 448\nLayerHeight = 720\n", 0 },
        { "history no graph",   "Name = X\nMyHist = HISTORY{int, 60}\n", 0 },
        { "invalid color",      "Name = X\nUser_Color = COLOR{ZZZZ}\n", 0 },
        { "unicode text",       "Name = X\nTEXT{\"Cześć świat\",0,0,0,0xFFFF}\n", 0 },
        { "negative coords",    "Name = X\nTEXT{\"hi\",-100,-200,0,0xFFFF}\n", 0 },
        { "color box",
          "Name = X\nMyColor = COLOR{0xF00F}\nBOX{0,0,100,100,MyColor}\n", 0 },
    };

    for (auto& c : cases) {
        size_t len = c.len ? c.len : std::strlen(c.src);

        smd::Document doc;
        bool loaded = doc.LoadFromMemory(c.src, len);
        (void)loaded;
        if (loaded) {
            bool compiled = doc.Compile();
            if (compiled) {
                RenderCtx ctx;
                (void)doc.Evaluate(render_cb, &ctx);
            }
        }
        doc.Free();

        // Re-use after Free.
        bool loaded2 = doc.LoadFromMemory(c.src, len);
        (void)loaded2;
        if (loaded2) (void)doc.Compile();

        ++g_pass;
        std::fprintf(stdout, "  malformed[%s]: ok\n", c.label);
    }

    // Buffer with embedded null bytes – LoadFromMemory takes an explicit size.
    {
        const char buf[] = "Name = X\0TEXT{\"hi\",0,0,0,0xFFFF}\n";
        smd::Document doc;
        bool loaded = doc.LoadFromMemory(buf, sizeof(buf) - 1);
        (void)loaded;
        if (loaded) (void)doc.Compile();
        ++g_pass;
        std::fprintf(stdout, "  malformed[null bytes middle]: ok\n");
    }
}

// ---------------------------------------------------------------------------
// Test 4: static Peek helpers
// ---------------------------------------------------------------------------
static void test_peek(const fs::path& modes_dir) {
    for (auto& e : fs::recursive_directory_iterator(modes_dir)) {
        if (!e.is_regular_file() || e.path().extension() != ".smd") continue;

        smd::Document::PeekInfo pi;
        bool ok = smd::Document::Peek(e.path().c_str(), pi);
        EXPECT(ok, ("Peek failed for " + e.path().string()).c_str());
        if (ok)
            std::fprintf(stdout, "  peek[%s]: name='%s' w=%lld h=%lld\n",
                         e.path().filename().c_str(), pi.name.c_str(),
                         (long long)pi.layerWidth, (long long)pi.layerHeight);

        // PeekFromMemory with the same content should give the same name.
        std::string content = slurp(e.path());
        smd::Document::PeekInfo pi2;
        bool ok2 = smd::Document::PeekFromMemory(
                       content.data(), content.size(), pi2);
        EXPECT(ok2, ("PeekFromMemory failed for " + e.path().string()).c_str());
        if (ok && ok2)
            EXPECT(pi.name == pi2.name,
                   "Peek and PeekFromMemory must return same name");
    }
}

// ---------------------------------------------------------------------------
// Test 5: ClearDimsMeasureCache doesn't crash on fresh / post-eval docs
// ---------------------------------------------------------------------------
static void test_clear_dims_cache(const fs::path& modes_dir) {
    fs::path first_smd;
    for (auto& e : fs::recursive_directory_iterator(modes_dir))
        if (e.is_regular_file() && e.path().extension() == ".smd")
            { first_smd = e.path(); break; }
    if (first_smd.empty()) return;

    const int64_t fps = 30;
    smd::Document doc;

    doc.ClearDimsMeasureCache();   // on a never-loaded document
    ++g_pass;

    doc.LoadFromFile(first_smd.c_str());
    doc.BindInt64("FPS_int", &fps);
    doc.Compile();

    RenderCtx ctx;
    doc.Evaluate(render_cb, &ctx);
    doc.ClearDimsMeasureCache();   // after evaluate
    ++g_pass;

    RenderCtx ctx2;
    bool ev2 = doc.Evaluate(render_cb, &ctx2);
    EXPECT(ev2, "Evaluate after ClearDimsMeasureCache");
}

// ---------------------------------------------------------------------------
// Test 6: GetFileHash is deterministic for the same content
// ---------------------------------------------------------------------------
static void test_file_hash(const fs::path& modes_dir) {
    fs::path first_smd;
    for (auto& e : fs::recursive_directory_iterator(modes_dir))
        if (e.is_regular_file() && e.path().extension() == ".smd")
            { first_smd = e.path(); break; }
    if (first_smd.empty()) return;

    smd::Document doc1, doc2;
    doc1.LoadFromFile(first_smd.c_str());
    doc2.LoadFromFile(first_smd.c_str());
    uint32_t h1 = doc1.GetFileHash();
    uint32_t h2 = doc2.GetFileHash();
    EXPECT(h1 == h2, "GetFileHash must be deterministic");
    EXPECT(h1 != 0,  "GetFileHash must be non-zero for non-empty file");
}

// ---------------------------------------------------------------------------
// Test 7: Locale RecordCallback is invoked during Load
// ---------------------------------------------------------------------------
static void test_locale_callback(const fs::path& modes_dir) {
    fs::path first_smd;
    for (auto& e : fs::recursive_directory_iterator(modes_dir))
        if (e.is_regular_file() && e.path().extension() == ".smd")
            { first_smd = e.path(); break; }
    if (first_smd.empty()) return;

    bool cb_called = false;
    smd::Document doc;
    doc.SetRecordCallback(
        [](std::string& out, void* user) {
            *static_cast<bool*>(user) = true;
            out = "PL-PL";
        },
        &cb_called
    );
    doc.LoadFromFile(first_smd.c_str());
    EXPECT(cb_called, "RecordCallback must be invoked during Load");

    // PeekName with locale
    std::string name;
    bool ok = smd::Document::PeekName(first_smd.c_str(), name, "PL-PL");
    EXPECT(ok, "PeekName with locale failed");
}

// ---------------------------------------------------------------------------
// Test 8: Reset(freeze=true) then Evaluate
// ---------------------------------------------------------------------------
static void test_reset_freeze(const fs::path& modes_dir) {
    fs::path first_smd;
    for (auto& e : fs::recursive_directory_iterator(modes_dir))
        if (e.is_regular_file() && e.path().extension() == ".smd")
            { first_smd = e.path(); break; }
    if (first_smd.empty()) return;

    const int64_t fps = 60;
    smd::Document doc;
    doc.LoadFromFile(first_smd.c_str());
    doc.BindInt64("FPS_int", &fps);
    doc.Compile();

    RenderCtx ctx1;
    doc.Evaluate(render_cb, &ctx1);

    doc.Reset(/*freeze=*/true);

    RenderCtx ctx2;
    bool ev = doc.Evaluate(render_cb, &ctx2);
    EXPECT(ev, "Evaluate after Reset(freeze=true)");
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    fs::path modes_dir = (argc > 1) ? argv[1] : "modes";

    std::fprintf(stdout, "=== smd_parser sanitizer tests ===\n");
    std::fprintf(stdout, "modes dir: %s\n\n", modes_dir.c_str());

    std::fprintf(stdout, "[1] fixture files\n");
    test_fixture_files(modes_dir);

    std::fprintf(stdout, "\n[2] reload\n");
    test_reload(modes_dir);

    std::fprintf(stdout, "\n[3] malformed inputs\n");
    test_malformed();

    std::fprintf(stdout, "\n[4] Peek helpers\n");
    test_peek(modes_dir);

    std::fprintf(stdout, "\n[5] ClearDimsMeasureCache\n");
    test_clear_dims_cache(modes_dir);

    std::fprintf(stdout, "\n[6] GetFileHash\n");
    test_file_hash(modes_dir);

    std::fprintf(stdout, "\n[7] locale callback\n");
    test_locale_callback(modes_dir);

    std::fprintf(stdout, "\n[8] Reset(freeze)\n");
    test_reset_freeze(modes_dir);

    std::fprintf(stdout, "\n=== results: %d passed, %d failed ===\n",
                 g_pass, g_fail);
    return (g_fail > 0) ? 1 : 0;
}
