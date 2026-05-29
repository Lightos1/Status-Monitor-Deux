#pragma once
#include "smse_types.hpp"
#include "smse_parser.hpp"
#include <switch.h>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <span>
#include "slre.h"
#include <filesystem>
#include <cstring>


// ---------------------------------------------------------------------------
// libnx plumbing
// ---------------------------------------------------------------------------

struct ServiceExtensions {
    Result      rc;
    std::string serviceName;
    Service     service;
};

extern std::list<ServiceExtensions> serviceExt;

static inline Result connectToService(const std::string& name) {
    SmServiceName encoded = smEncodeName(name.c_str());
    Handle h;
    bool running = R_FAILED(smRegisterService(&h, encoded, false, 1));
    if (!running) {
        smUnregisterService(encoded);
        Service s{}; serviceExt.emplace_back(0, name, s); return 1;
    }
    Service s{};
    Result rc = smGetService(&s, name.c_str());
    serviceExt.emplace_back(rc, name, s);
    return rc;
}

static inline Result serviceCall(Service* s, u32 cmd,
                                 void* out, size_t out_size, bool buffer) {
    if (buffer) {
        return serviceDispatch(s, cmd,
            .buffer_attrs = { SfBufferAttr_HipcAutoSelect | SfBufferAttr_Out },
            .buffers = {{out, out_size}});
    }
    return serviceDispatchImpl(s, cmd, nullptr, 0, out, out_size, (SfDispatchParams){});
}

// ---------------------------------------------------------------------------
// Conversion helpers
// Read from raw bytes in the wire type, produce s64 / float / double.
// ---------------------------------------------------------------------------

static inline bool isIntType(SmseValueType t) noexcept {
    using V = SmseValueType;
    return t==V::u8_||t==V::u16_||t==V::u32_||t==V::u64_||
           t==V::s8_||t==V::s16_||t==V::s32_||t==V::s64_;
}

static inline s64 extractInt(const u8* src, SmseValueType t) noexcept {
    switch (t) {
        case SmseValueType::u8_:  { u8  v; __builtin_memcpy(&v, src, 1); return (s64)v; }
        case SmseValueType::u16_: { u16 v; __builtin_memcpy(&v, src, 2); return (s64)v; }
        case SmseValueType::u32_: { u32 v; __builtin_memcpy(&v, src, 4); return (s64)v; }
        case SmseValueType::u64_: { u64 v; __builtin_memcpy(&v, src, 8); return (s64)v; }
        case SmseValueType::s8_:  { s8  v; __builtin_memcpy(&v, src, 1); return (s64)v; }
        case SmseValueType::s16_: { s16 v; __builtin_memcpy(&v, src, 2); return (s64)v; }
        case SmseValueType::s32_: { s32 v; __builtin_memcpy(&v, src, 4); return (s64)v; }
        case SmseValueType::s64_: { s64 v; __builtin_memcpy(&v, src, 8); return  v; }
        default: return 0;
    }
}

inline std::size_t safe_strlen(const char* buf, std::size_t max_size) {
    const char* end = std::find(buf, buf + max_size, '\0');
    return std::distance(buf, end);
}

// ---------------------------------------------------------------------------
// Accessor
//
// ptr always points to:
//   s64    — for any integer source type (u8..u64, s8..s64)
//   float  — for f32
//   double — for f64
//   char   — for buffer_char (C string)
//
// srcType tells you the original wire type from the .smse file.
// ---------------------------------------------------------------------------

struct SmseFieldAccessor {
    SmseValueType srcType;
    const void*   ptr;      // stable pointer; never moves after smseLoadFolder()

    bool isInt()    const noexcept { return isIntType(srcType); }
    bool isFloat()  const noexcept { return srcType == SmseValueType::f32_; }
    bool isDouble() const noexcept { return srcType == SmseValueType::f64_; }
    bool isString() const noexcept { return srcType == SmseValueType::buffer_char; }

    // Typed pointer access — choose based on isInt/isFloat/isDouble/isString
    const s64*    intPtr() const noexcept { return static_cast<const s64*>(ptr); }
    const float*  f32Ptr() const noexcept { return static_cast<const float*>(ptr); }
    const double* f64Ptr() const noexcept { return static_cast<const double*>(ptr); }
    const char*   strPtr() const noexcept { return static_cast<const char*>(ptr); }

    // Value shorthands
    s64         asInt()    const noexcept { return *intPtr(); }
    float       asFloat()  const noexcept { return *f32Ptr(); }
    double      asDouble() const noexcept { return *f64Ptr(); }
    const char* asString() const noexcept { return  strPtr(); }
};

// ---------------------------------------------------------------------------
// Public variable entry — one per struct field / scalar output / string output
// ---------------------------------------------------------------------------

struct SmseVarEntry {
    std::string       name;       // field name or command name
    std::string       service;    // owning service
    SmseFieldAccessor accessor;   // stable; bind to other threads freely
};

// ---------------------------------------------------------------------------
// Public service entry
// ---------------------------------------------------------------------------

struct SmseServiceEntry {
    std::string            name;
    bool                   connected  = false;
    std::vector<SmseError> initErrors;   // connection + assert; fixed after load
    std::vector<SmseError>* execErrors; // live; cleared+filled each smseExecuteAll()
};

// ---------------------------------------------------------------------------
// Internal storage
//
// Separation of concerns:
//   rawData  — scratch buffer handed to serviceCall (wire size)
//   *Storage — converted values that accessors point into (stable pointers)
// ---------------------------------------------------------------------------

// One entry in the field conversion map for a buffer slot
struct SmseConvertEntry {
    size_t        srcOffset;
    SmseValueType srcType;
    size_t        storageIdx;   // index into the relevant storage vector
    // which storage to use (mutually exclusive):
    bool isF32 = false;
    bool isF64 = false;
    // else: integer -> intStorage
};

struct SmseBufferSlot {
    std::vector<u8>              rawData;     // wire-size scratch; never exposed
    std::vector<SmseConvertEntry> fieldMap;   // how to convert rawData -> storage
    std::vector<s64>             intStorage;  // all integer fields, as s64
    std::vector<float>           f32Storage;
    std::vector<double>          f64Storage;
    bool isChar = false;  // if true: rawData is the string; no fieldMap used

    // Called after every successful serviceCall
    void convert() noexcept {
        if (isChar) return;  // rawData is the output itself
        for (auto& fe : fieldMap) {
            const u8* src = rawData.data() + fe.srcOffset;
            if (fe.isF32) {
                float v; __builtin_memcpy(&v, src, 4); f32Storage[fe.storageIdx] = v;
            } else if (fe.isF64) {
                double v; __builtin_memcpy(&v, src, 8); f64Storage[fe.storageIdx] = v;
            } else {
                intStorage[fe.storageIdx] = extractInt(src, fe.srcType);
            }
        }
    }
};

struct SmseScalarSlot {
    SmseValueType srcType;
    s64    intVal = 0;
    float  f32Val = 0.0f;
    double f64Val = 0.0;

    // Write from a raw wire buffer (1-8 bytes at ptr)
    void write(const u8* src) noexcept {
        if (srcType == SmseValueType::f32_) {
            __builtin_memcpy(&f32Val, src, 4);
        } else if (srcType == SmseValueType::f64_) {
            __builtin_memcpy(&f64Val, src, 8);
        } else {
            intVal = extractInt(src, srcType);
        }
    }

    // Stable pointer for the accessor
    const void* ptr() const noexcept {
        if (srcType == SmseValueType::f32_) return &f32Val;
        if (srcType == SmseValueType::f64_) return &f64Val;
        return &intVal;
    }
};

// ---------------------------------------------------------------------------
// Per-service runtime
// ---------------------------------------------------------------------------

struct SmseServiceRuntime {
    std::string  serviceName;
    Service*     svc = nullptr;

    std::unordered_map<u32,         SmseCommandDesc> cmds;
    std::unordered_map<std::string, SmseStructDesc>  structs;
    std::unordered_map<std::string, SmseScalarSlot>  scalarOutputs;
    std::unordered_map<std::string, SmseBufferSlot>  bufferOutputs;

    std::vector<std::string>       execOrder;
    std::unordered_set<std::string> execSeen;

    std::vector<SmseError> execErrors;
};

// ---------------------------------------------------------------------------
// Loader
// ---------------------------------------------------------------------------

class SmseLoader {
public:
    std::vector<SmseError> loadErrors;

    bool addParsedFile(SmseParsedFile&& pf) {
        for (auto& [sname, sd] : pf.structs) {
            auto& ex = _pendingStructs[sname];
            if (ex.name.empty()) {
                ex = sd;
            } else {
                if (sd.bufSize && ex.bufSize && sd.bufSize != ex.bufSize) {
                    loadErrors.push_back(SmseError::make(
                        SmseErrorCode::ParseStructSizeConflict, pf.sourceFile,
                        fmt("Struct '%s' size conflict: %zu vs %zu",
                                    sname.c_str(), ex.bufSize, sd.bufSize)));
                    return false;
                }
                if (sd.bufSize) ex.bufSize = sd.bufSize;
                for (auto& f : sd.fields) ex.fields.push_back(f);
            }
        }
        _pendingFiles.push_back(std::move(pf));
        return true;
    }

    std::vector<SmseError> finalize() {
        std::vector<SmseError> errors;

        std::unordered_map<std::string, std::vector<SmseParsedFile*>> byService;
        for (auto& pf : _pendingFiles)
            byService[pf.serviceName].push_back(&pf);

        for (auto& [svcName, files] : byService) {
            SmseServiceEntry sentry;
            sentry.name = svcName;
            sentry.execErrors = nullptr;

            // --- Connect once ---
            auto* existing = _findService(svcName);
            if (!existing) {
                Result rc = connectToService(svcName);
                if (R_FAILED(rc)) {
                    for (auto* pf : files) {
                        auto e = SmseError::make(SmseErrorCode::ServiceConnectFailed,
                            pf->sourceFile,
                            fmt("connectToService('%s') failed", svcName.c_str()), rc);
                        sentry.initErrors.push_back(e);
                        errors.push_back(e);
                    }
                    _serviceEntries.push_back(std::move(sentry));
                    continue;
                }
                existing = _findService(svcName);
            }
            if (!existing) {
                auto e = SmseError::make(SmseErrorCode::ServiceConnectFailed,
                    files[0]->sourceFile,
                    fmt("'%s' missing from serviceExt after connect", svcName.c_str()));
                sentry.initErrors.push_back(e);
                errors.push_back(e);
                _serviceEntries.push_back(std::move(sentry));
                continue;
            }
            sentry.connected = true;

            // --- Build runtime ---
            auto& rt = _runtimes[svcName];
            rt.serviceName = svcName;
            rt.svc = &existing->service;

            for (auto* pf : files) {
                for (auto& cmd : pf->cmds)
                    rt.cmds.emplace(cmd.cmdId, cmd);
                for (auto& [sname, sd] : pf->structs) {
                    auto& dst = rt.structs[sname];
                    if (dst.name.empty()) dst = sd;
                    else {
                        if (sd.bufSize) dst.bufSize = sd.bufSize;
                        for (auto& f : sd.fields) dst.fields.push_back(f);
                    }
                }
                for (auto& ec : pf->execCmds)
                    if (rt.execSeen.insert(ec).second)
                        rt.execOrder.push_back(ec);
            }

            // --- Allocate output slots ---
            // All allocations happen here; pointers are stable from this point on.
            for (auto& [cmdId, cmd] : rt.cmds) {
                if (cmd.isBuffer) {
                    auto& slot = rt.bufferOutputs[cmd.name];
                    slot.rawData.resize(cmd.bufSize, 0);
                    slot.isChar = cmd.bufIsChar;
                    if (!cmd.bufIsChar) {
                        // Build conversion map from the merged struct descriptor
                        auto it = rt.structs.find(cmd.bufStructName);
                        if (it != rt.structs.end()) {
                            size_t intIdx=0, f32Idx=0, f64Idx=0;
                            for (auto& field : it->second.fields) {
                                SmseConvertEntry fe;
                                fe.srcOffset  = field.offset;
                                fe.srcType    = field.type;
                                fe.isF32      = (field.type == SmseValueType::f32_);
                                fe.isF64      = (field.type == SmseValueType::f64_);
                                if      (fe.isF32) fe.storageIdx = f32Idx++;
                                else if (fe.isF64) fe.storageIdx = f64Idx++;
                                else               fe.storageIdx = intIdx++;
                                slot.fieldMap.push_back(fe);
                            }
                            slot.intStorage.resize(intIdx, 0);
                            slot.f32Storage.resize(f32Idx, 0.0f);
                            slot.f64Storage.resize(f64Idx, 0.0);
                        }
                    }
                } else {
                    rt.scalarOutputs[cmd.name] = SmseScalarSlot{ cmd.outputType };
                }
            }

            // --- Build _allVars (pointers are now stable) ---

            // Struct fields
            for (auto& [sname, sd] : rt.structs) {
                for (auto& [cmdId, cmd] : rt.cmds) {
                    if (!cmd.isBuffer || cmd.bufIsChar) continue;
                    if (cmd.bufStructName != sname) continue;
                    auto it = rt.bufferOutputs.find(cmd.name);
                    if (it == rt.bufferOutputs.end()) continue;
                    auto& slot = it->second;
                    for (size_t i = 0; i < slot.fieldMap.size(); ++i) {
                        auto& fe = slot.fieldMap[i];
                        const void* ptr =
                            fe.isF32 ? (const void*)&slot.f32Storage[fe.storageIdx] :
                            fe.isF64 ? (const void*)&slot.f64Storage[fe.storageIdx] :
                                       (const void*)&slot.intStorage[fe.storageIdx];
                        _allVars.push_back({
                            sd.fields[i].name, svcName,
                            SmseFieldAccessor{ fe.srcType, ptr }
                        });
                    }
                    break;
                }
            }

            // Scalar exec outputs
            for (auto& execName : rt.execOrder) {
                for (auto& [cmdId, cmd] : rt.cmds) {
                    if (cmd.name != execName || cmd.isBuffer) continue;
                    auto it = rt.scalarOutputs.find(cmd.name);
                    if (it == rt.scalarOutputs.end()) break;
                    // ptr() returns pointer into SmseScalarSlot's stable member
                    _allVars.push_back({
                        cmd.name, svcName,
                        SmseFieldAccessor{ cmd.outputType, it->second.ptr() }
                    });
                    break;
                }
            }

            // Char-buffer exec outputs
            for (auto& execName : rt.execOrder) {
                for (auto& [cmdId, cmd] : rt.cmds) {
                    if (cmd.name != execName || !cmd.isBuffer || !cmd.bufIsChar) continue;
                    auto it = rt.bufferOutputs.find(cmd.name);
                    if (it == rt.bufferOutputs.end()) break;
                    _allVars.push_back({
                        cmd.name, svcName,
                        SmseFieldAccessor{ SmseValueType::buffer_char,
                                           it->second.rawData.data() }
                    });
                    break;
                }
            }

            // --- Asserts ---
            for (auto* pf : files) {
                for (auto& asrt : pf->asserts) {
                    auto e = _runAssert(rt, asrt, pf->sourceFile);
                    if (!e.ok()) {
                        sentry.initErrors.push_back(e);
                        errors.push_back(e);
                    }
                }
            }

            sentry.execErrors = &rt.execErrors;
            _serviceEntries.push_back(std::move(sentry));
        }

        return errors;
    }

    void executeAll() {
        for (auto& [svcName, rt] : _runtimes) {
            rt.execErrors.clear();
            for (auto& cmdName : rt.execOrder)
                _execCommand(rt, cmdName);
        }
    }

    std::span<const SmseVarEntry>     allVars()     const noexcept { return _allVars; }
    std::span<const SmseServiceEntry> allServices() const noexcept { return _serviceEntries; }

private:
    std::vector<SmseParsedFile>                         _pendingFiles;
    std::unordered_map<std::string, SmseStructDesc>     _pendingStructs;
    std::unordered_map<std::string, SmseServiceRuntime> _runtimes;
    std::vector<SmseVarEntry>                           _allVars;
    std::vector<SmseServiceEntry>                       _serviceEntries;

    ServiceExtensions* _findService(const std::string& name) {
        for (auto& se : serviceExt)
            if (se.serviceName == name) return &se;
        return nullptr;
    }

    void _execCommand(SmseServiceRuntime& rt, const std::string& cmdName) {
        for (auto& [cmdId, cmd] : rt.cmds) {
            if (cmd.name != cmdName) continue;
            Result rc;
            if (cmd.isBuffer) {
                auto& slot = rt.bufferOutputs[cmd.name];
                rc = serviceCall(rt.svc, cmd.cmdId,
                                 slot.rawData.data(), slot.rawData.size(), true);
                if (R_SUCCEEDED(rc)) slot.convert();
            } else {
                auto& slot = rt.scalarOutputs[cmd.name];
                u64 tmp = 0;
                rc = serviceCall(rt.svc, cmd.cmdId, &tmp,
                                 valueTypeSize(cmd.outputType), false);
                if (R_SUCCEEDED(rc))
                    slot.write(reinterpret_cast<const u8*>(&tmp));
            }
            if (R_FAILED(rc))
                rt.execErrors.push_back(SmseError::make(
                    SmseErrorCode::ExecCommandFailed, rt.serviceName,
                    fmt("Command '%s' (id=%u) failed", cmd.name.c_str(), cmd.cmdId), rc));
            return;
        }
        rt.execErrors.push_back(SmseError::make(
            SmseErrorCode::ExecCommandNotFound, rt.serviceName,
            fmt("Exec command '%s' not found in cmds", cmdName.c_str())));
    }

    SmseError _runAssert(SmseServiceRuntime& rt,
                         const SmseAssertDesc& asrt,
                         const std::string& src)
    {
        const SmseCommandDesc* cmd = nullptr;
        for (auto& [id, c] : rt.cmds)
            if (c.name == asrt.cmdName) { cmd = &c; break; }
        if (!cmd)
            return SmseError::make(SmseErrorCode::AssertUnknownCommand, src,
                                   fmt("Assert cmd '%s' not found", asrt.cmdName.c_str()));

        // Determine if this is a numeric check or a string regex check
        bool isNumericOp = (asrt.op != SmseAssertOp::EQ_REGEX && asrt.op != SmseAssertOp::NE_REGEX);

        if (isNumericOp) {
            if (cmd->isBuffer)
                return SmseError::make(SmseErrorCode::AssertBadOutputType, src,
                    fmt("Assert numeric op on buffer cmd '%s'", asrt.cmdName.c_str()));
            
            u64 tmp = 0;
            Result rc = serviceCall(rt.svc, cmd->cmdId, &tmp,
                                    valueTypeSize(cmd->outputType), false);
            if (R_FAILED(rc))
                return SmseError::make(SmseErrorCode::ExecCommandFailed, src,
                    fmt("Assert call '%s' failed", asrt.cmdName.c_str()), rc);
            
            s64 val = extractInt(reinterpret_cast<const u8*>(&tmp), cmd->outputType);
            s64 expected = static_cast<s64>(asrt.numericValue);
            
            bool passed = false;
            const char* opStr = "";
            switch (asrt.op) {
                case SmseAssertOp::EQ: passed = (val == expected); opStr = "=="; break;
                case SmseAssertOp::NE: passed = (val != expected); opStr = "!="; break;
                case SmseAssertOp::LT: passed = (val <  expected); opStr = "<";  break;
                case SmseAssertOp::LE: passed = (val <= expected); opStr = "<="; break;
                case SmseAssertOp::GT: passed = (val >  expected); opStr = ">";  break;
                case SmseAssertOp::GE: passed = (val >= expected); opStr = ">="; break;
                default: break;
            }

            if (!passed)
                return SmseError::make(SmseErrorCode::AssertVersionTooLow, src,
                    fmt("'%s' = %lld, condition (%s %lld) failed",
                                asrt.cmdName.c_str(), (long long)val, opStr, (long long)expected));
        } else {
            if (!cmd->isBuffer || !cmd->bufIsChar)
                return SmseError::make(SmseErrorCode::AssertBadOutputType, src,
                    fmt("Assert regex on non-char cmd '%s'", asrt.cmdName.c_str()));
            
            std::vector<u8> tmp(cmd->bufSize, 0);
            Result rc = serviceCall(rt.svc, cmd->cmdId, tmp.data(), tmp.size(), true);
            if (R_FAILED(rc))
                return SmseError::make(SmseErrorCode::ExecCommandFailed, src,
                    fmt("Assert call '%s' failed", asrt.cmdName.c_str()), rc);
            
            std::string got(reinterpret_cast<char*>(tmp.data()),
                            safe_strlen(reinterpret_cast<char*>(tmp.data()), tmp.size()));
            
            // Anchor pattern for full-string match semantics (slre_match does substring search)
            std::string anchored = "^(" + asrt.regex + ")$";
            bool matched = slre_match(anchored.c_str(), got.c_str(),
                                      static_cast<int>(got.size()),
                                      nullptr, 0, 0) >= 0;

            // For EQ_REGEX, we want matched == true. For NE_REGEX, we want matched == false.
            bool passed = (asrt.op == SmseAssertOp::EQ_REGEX) ? matched : !matched;
            
            if (!passed) {
                const char* condStr = (asrt.op == SmseAssertOp::EQ_REGEX) ? "match" : "not match";
                return SmseError::make(SmseErrorCode::AssertVersionStringMismatch, src,
                    fmt("'%s' = '%s' expected to %s '%s'",
                                asrt.cmdName.c_str(), got.c_str(), condStr, asrt.regex.c_str()));
            }
        }
        
        return SmseError::success();
    }
};

// ---------------------------------------------------------------------------
// Global instance + public API
// ---------------------------------------------------------------------------

inline SmseLoader g_smse;

inline std::vector<SmseError> smseLoadFolder(const char* folderPath) {
    std::vector<SmseError> all;
    std::error_code ec;
    for (auto& entry : std::filesystem::directory_iterator(folderPath, ec)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".smse") continue;
        auto result = smse::parseFile(entry.path().string());
        if (auto* err = std::get_if<SmseError>(&result)) {
            all.push_back(std::move(*err)); continue;
        }
        if (!g_smse.addParsedFile(std::move(std::get<SmseParsedFile>(result))))
            for (auto& e : g_smse.loadErrors) all.push_back(e);
    }
    if (ec)
        all.push_back(SmseError::make(SmseErrorCode::ParseMalformedLine,
            folderPath, fmt("Cannot iterate folder: %s", ec.message().c_str())));
    for (auto& e : g_smse.finalize())    all.push_back(e);
    for (auto& e : g_smse.loadErrors)    all.push_back(e);
    return all;
}

// Every variable from every loaded service.
// Stable span; entries never move after smseLoadFolder() returns.
inline std::span<const SmseVarEntry> smseGetAllVars() noexcept {
    return g_smse.allVars();
}

// Every service with connection status + error context.
// execErrors pointer is live: refreshed each smseExecuteAll().
inline std::span<const SmseServiceEntry> smseGetAllServices() noexcept {
    return g_smse.allServices();
}

// Call from your execution thread on every tick.
inline void smseExecuteAll() {
    g_smse.executeAll();
}
