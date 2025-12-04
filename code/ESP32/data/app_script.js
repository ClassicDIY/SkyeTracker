const char app_script_js[] PROGMEM = R"rawliteral(
    try {
        const fields = getFormValues("#app_fields input, #app_fields select");
        const res = await fetch('/app_fields', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(fields)
        });
        console.log(await res.text());
        if (!res.ok) throw new Error('Save failed');
        status.innerHTML = '<span class="ok">APP Settings saved.</span>';
    } catch (e) {
        status.innerHTML = '<span class="err">' + e.message + '</span>';
    }
)rawliteral";
