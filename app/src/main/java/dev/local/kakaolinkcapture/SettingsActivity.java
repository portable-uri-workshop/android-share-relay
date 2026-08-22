package dev.local.kakaolinkcapture;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.TypedValue;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;

public final class SettingsActivity extends Activity {
    public static final String PREFERENCES_NAME = "capture_settings";
    public static final String KEY_USE_LINK = "use_deep_link_site";
    public static final String KEY_SENSITIVE_CLIPBOARD = "mark_clipboard_sensitive";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        SharedPreferences preferences = getSharedPreferences(PREFERENCES_NAME, MODE_PRIVATE);
        int padding = dp(24);

        LinearLayout content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        content.setPadding(padding, padding, padding, padding);

        TextView title = text(getString(R.string.settings_title), 26, true);
        content.addView(title);

        TextView introduction = text(getString(R.string.settings_intro), 15, false);
        introduction.setPadding(0, dp(8), 0, dp(20));
        content.addView(introduction);

        Switch linkSwitch = new Switch(this);
        linkSwitch.setText(R.string.use_link_label);
        linkSwitch.setTextSize(TypedValue.COMPLEX_UNIT_SP, 17);
        linkSwitch.setChecked(preferences.getBoolean(KEY_USE_LINK, false));
        linkSwitch.setOnCheckedChangeListener((button, checked) ->
                preferences.edit().putBoolean(KEY_USE_LINK, checked).apply());
        content.addView(linkSwitch, matchWrap());

        TextView linkSummary = text(getString(R.string.use_link_summary), 14, false);
        linkSummary.setPadding(0, dp(4), 0, dp(18));
        content.addView(linkSummary);

        Switch sensitiveSwitch = new Switch(this);
        sensitiveSwitch.setText(R.string.sensitive_label);
        sensitiveSwitch.setTextSize(TypedValue.COMPLEX_UNIT_SP, 17);
        sensitiveSwitch.setChecked(preferences.getBoolean(KEY_SENSITIVE_CLIPBOARD, false));
        sensitiveSwitch.setOnCheckedChangeListener((button, checked) ->
                preferences.edit().putBoolean(KEY_SENSITIVE_CLIPBOARD, checked).apply());
        content.addView(sensitiveSwitch, matchWrap());

        TextView sensitiveSummary = text(getString(R.string.sensitive_summary), 14, false);
        sensitiveSummary.setPadding(0, dp(4), 0, dp(18));
        content.addView(sensitiveSummary);

        TextView behavior = text(getString(R.string.capture_behavior), 14, false);
        content.addView(behavior);

        ScrollView scrollView = new ScrollView(this);
        scrollView.addView(content, matchWrap());
        setContentView(scrollView);
    }

    @Override
    protected void onStop() {
        super.onStop();
        // Settings are launcher-only. Do not leave this Activity underneath a
        // later share-capture task, where finishing the capture could reveal it.
        if (!isChangingConfigurations()) {
            finish();
        }
    }

    private TextView text(String value, int sizeSp, boolean bold) {
        TextView view = new TextView(this);
        view.setText(value);
        view.setTextSize(TypedValue.COMPLEX_UNIT_SP, sizeSp);
        if (bold) view.setTypeface(view.getTypeface(), android.graphics.Typeface.BOLD);
        view.setLineSpacing(0, 1.15f);
        return view;
    }

    private LinearLayout.LayoutParams matchWrap() {
        return new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
    }

    private int dp(int value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }
}
