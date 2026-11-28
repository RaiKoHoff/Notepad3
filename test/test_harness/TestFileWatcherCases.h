static WIN32_FILE_ATTRIBUTE_DATA Snapshot(DWORD size) {
    WIN32_FILE_ATTRIBUTE_DATA snapshot = {};
    snapshot.nFileSizeLow = size;
    return snapshot;
}

static DWORD fileSize;
static bool fileExists = true;
static bool registrationFails;
static bool rearmFails;
static bool waitFails;
static bool changeOnPoll;
static DWORD firstTimeout;
static bool replaceBaselineDuringProbe;
static unsigned queries;
static unsigned attempts;
static UINT postedMessage;
static WPARAM postedGeneration;
static LPARAM postedVersion;
static unsigned failedPosts;
static unsigned waits;
static HANDLE observedCancel;
static HANDLE queryStarted;
static HANDLE releaseQuery;
static UINT timerInterval;

static bool TestGetAttributes(HPATHL path, GET_FILEEX_INFO_LEVELS, LPVOID data) {
    ++queries;
    DWORD const size = fileSize;
    if (replaceBaselineDuringProbe) {
        replaceBaselineDuringProbe = false;
        fileSize = 20;
        auto const baseline = Snapshot(20);
        SetFileObservationData(true, &baseline);
    }
    if (queryStarted) {
        assert(wcscmp(path, L"owned") == 0);
        SetEvent(queryStarted);
        assert(WaitForSingleObject(releaseQuery, 5000) == WAIT_OBJECT_0);
    }
    *static_cast<WIN32_FILE_ATTRIBUTE_DATA*>(data) = Snapshot(size);
    return fileExists;
}

static HANDLE TestFindFirst(LPCWSTR path, BOOL, DWORD) {
    assert(wcscmp(path, L"directory") == 0);
    return registrationFails ? INVALID_HANDLE_VALUE : CreateEvent(NULL, FALSE, rearmFails, NULL);
}
static BOOL TestFindNext(HANDLE) { return !rearmFails; }
static BOOL TestPostMessage(HWND hwnd, UINT message, WPARAM generation, LPARAM version) {
    assert(hwnd == Globals.hwndMain);
    ++attempts;
    if (failedPosts) {
        --failedPosts;
        return FALSE;
    }
    postedMessage = message;
    postedGeneration = generation;
    postedVersion = version;
    if (observedCancel) { SetEvent(observedCancel); }
    return TRUE;
}
static UINT_PTR TestSetTimer(HWND, UINT_PTR id, UINT interval, TIMERPROC) {
    timerInterval = interval;
    return id;
}
static BOOL TestKillTimer(HWND, UINT_PTR id) {
    if (id == ID_WATCHTIMER) { timerInterval = 0; }
    return TRUE;
}
#undef WaitForMultipleObjects
static DWORD TestWaitForMultipleObjects(DWORD count, const HANDLE* handles, BOOL all, DWORD timeout) {
    assert(++waits < 20);
    if (waits == 1) { firstTimeout = timeout; }
    if (waitFails) { return WAIT_FAILED; }
    if (changeOnPoll && timeout != INFINITE) {
        fileSize = 11;
        changeOnPoll = false;
    }
    return WaitForMultipleObjects(count, handles, all, timeout == INFINITE ? 250 : timeout);
}

static FileWatchRun* NewRun(bool immediate, bool periodic = false) {
    auto* const run = static_cast<FileWatchRun*>(GlobalAlloc(GPTR, sizeof(FileWatchRun)));
    assert(run && !s_fileWatch.run);
    run->refs = 2;
    run->hwnd = Globals.hwndMain;
    run->file = L"owned";
    run->directory = L"directory";
    run->cancel = CreateEvent(NULL, TRUE, FALSE, NULL);
    run->baselineChanged = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(run->cancel && run->baselineChanged);
    run->generation = s_fileWatch.generation;
    run->immediate = immediate;
    run->periodic = periodic;
    run->interval = 10;
    s_fileWatch.run = run;
    attempts = waits = 0;
    postedMessage = 0;
    return run;
}

static DWORD WINAPI RunObserver(LPVOID run) { return FileChangeObserver(run); }

int main() {
    StopFileWatch(Globals.hwndMain);
    auto const baseline = Snapshot(10);
    fileSize = 11;
    SetFileObservationData(true, &baseline);
    replaceBaselineDuringProbe = true;
    ProbeFileWatch(L"owned", s_fileWatch.generation);
    assert(queries == 2 && !IsFileChangedFlagSet());

    // A blocked probe may outlive Stop; it must not reassert discarded flags.
    fileExists = false;
    queryStarted = CreateEvent(NULL, TRUE, FALSE, NULL);
    releaseQuery = CreateEvent(NULL, TRUE, FALSE, NULL);
    auto* run = NewRun(true);
    ULONG const retired = run->generation;
    HANDLE const worker = CreateThread(NULL, 0, RunObserver, run, 0, NULL);
    assert(worker && WaitForSingleObject(queryStarted, 5000) == WAIT_OBJECT_0);
    StopFileWatch(Globals.hwndMain);
    SetFileObservationData(true, &baseline);
    assert(WaitForSingleObject(worker, 0) == WAIT_TIMEOUT);
    SetEvent(releaseQuery);
    assert(WaitForSingleObject(worker, 5000) == WAIT_OBJECT_0);
    assert(!IsFileChangedFlagSet() && !IsFileDeletedFlagSet() && !postedMessage);
    assert(IsStaleFileWatchMessage(retired, s_fileWatch.baselineVersion));
    CloseHandle(worker);
    CloseHandle(queryStarted);
    CloseHandle(releaseQuery);
    queryStarted = NULL;

    // The disk has diverged from the loaded snapshot before registration.
    // Reject the first wakeup and retry with no directory event.
    fileExists = true;
    fileSize = 11;
    run = NewRun(true);
    observedCancel = run->cancel;
    failedPosts = 1;
    assert(FileChangeObserver(run) == 0);
    assert(attempts == 2 && firstTimeout == 100 && postedMessage == WM_FILECHANGEDNOTIFY);
    assert(!IsStaleFileWatchMessage(postedGeneration, postedVersion));
    assert(!IsFileChangedFlagSet());
    StopFileWatch(Globals.hwndMain);
    assert(IsStaleFileWatchMessage(postedGeneration, postedVersion));

    // A file already missing when the watcher starts is still observed.
    SetFileObservationData(true, &baseline);
    fileExists = false;
    run = NewRun(true);
    observedCancel = run->cancel;
    assert(FileChangeObserver(run) == 0);
    assert(postedMessage == WM_FILECHANGEDNOTIFY && IsFileDeletedFlagSet());
    StopFileWatch(Globals.hwndMain);
    fileExists = true;

    // BOTH and registration/rearm failure still poll on the worker.
    for (int mode = 0; mode < 3; ++mode) {
        registrationFails = mode == 1;
        rearmFails = mode == 2;
        changeOnPoll = true;
        fileSize = 10;
        SetFileObservationData(true, &baseline);
        run = NewRun(false, mode == 0);
        observedCancel = run->cancel;
        assert(FileChangeObserver(run) == 0);
        assert(postedMessage == WM_FILECHANGEDNOTIFY && timerInterval == 0);
        StopFileWatch(Globals.hwndMain);
    }
    registrationFails = rearmFails = false;

    // A wait failure hands off to the UI only after its wakeup is queued.
    fileSize = 10;
    SetFileObservationData(true, &baseline);
    run = NewRun(true);
    observedCancel = run->cancel;
    waitFails = true;
    failedPosts = 1;
    assert(FileChangeObserver(run) == 1);
    assert(attempts == 2 && firstTimeout == INFINITE && postedMessage == WM_FILEWATCHFAILED);
    HandleFileWatchFailure(postedGeneration - 1, 0);
    assert(timerInterval == 0);
    HandleFileWatchFailure(postedGeneration, 0);
    assert(timerInterval == 500 && !s_fileWatch.run);
    StopFileWatch(Globals.hwndMain);

    puts("File watcher tests passed");
    return 0;
}
