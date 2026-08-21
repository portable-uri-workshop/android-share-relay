#include <android/native_activity.h>
#include <jni.h>

#include <cstring>

namespace {

constexpr char kRedirectPrefix[] = "https://portable-uri-workshop.github.io/#to=";
constexpr char kPreferencesName[] = "capture_settings";
constexpr char kUseLinkPreference[] = "use_deep_link_site";
constexpr char kSensitivePreference[] = "mark_clipboard_sensitive";
constexpr char kSensitiveExtra[] = "android.content.extra.IS_SENSITIVE";
constexpr jsize kMaximumUriLength = 32768;

void clear_pending_exception(JNIEnv* env) {
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
}

bool string_equals(JNIEnv* env, jstring value, const char* expected) {
    if (value == nullptr) {
        return false;
    }
    const char* characters = env->GetStringUTFChars(value, nullptr);
    if (characters == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return false;
    }
    const bool matches = std::strcmp(characters, expected) == 0;
    env->ReleaseStringUTFChars(value, characters);
    return matches;
}

void show_toast(JNIEnv* env, jobject activity, const char* text) {
    jclass toast_class = env->FindClass("android/widget/Toast");
    jmethodID make_text = toast_class == nullptr
        ? nullptr
        : env->GetStaticMethodID(
              toast_class,
              "makeText",
              "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
    if (make_text == nullptr) {
        clear_pending_exception(env);
        return;
    }

    jstring message = env->NewStringUTF(text);
    jobject toast = env->CallStaticObjectMethod(toast_class, make_text, activity, message, 0);
    if (toast == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return;
    }

    jclass toast_instance_class = env->GetObjectClass(toast);
    jmethodID show = toast_instance_class == nullptr
        ? nullptr
        : env->GetMethodID(toast_instance_class, "show", "()V");
    if (show != nullptr) {
        env->CallVoidMethod(toast, show);
        clear_pending_exception(env);
    }
}

void finish(JNIEnv* env, jobject activity) {
    jclass activity_class = env->GetObjectClass(activity);
    jmethodID finish_method = activity_class == nullptr
        ? nullptr
        : env->GetMethodID(activity_class, "finish", "()V");
    if (finish_method != nullptr) {
        env->CallVoidMethod(activity, finish_method);
        clear_pending_exception(env);
    }
}

jstring get_valid_intent_data(JNIEnv* env, jobject activity) {
    jclass activity_class = env->GetObjectClass(activity);
    jmethodID get_intent = activity_class == nullptr
        ? nullptr
        : env->GetMethodID(activity_class, "getIntent", "()Landroid/content/Intent;");
    if (get_intent == nullptr) {
        clear_pending_exception(env);
        return nullptr;
    }

    jobject intent = env->CallObjectMethod(activity, get_intent);
    if (intent == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return nullptr;
    }

    jclass intent_class = env->FindClass("android/content/Intent");
    jmethodID get_action = intent_class == nullptr
        ? nullptr
        : env->GetMethodID(intent_class, "getAction", "()Ljava/lang/String;");
    jmethodID get_data = intent_class == nullptr
        ? nullptr
        : env->GetMethodID(intent_class, "getData", "()Landroid/net/Uri;");
    jmethodID get_data_string = intent_class == nullptr
        ? nullptr
        : env->GetMethodID(intent_class, "getDataString", "()Ljava/lang/String;");
    if (get_action == nullptr || get_data == nullptr || get_data_string == nullptr) {
        clear_pending_exception(env);
        return nullptr;
    }

    auto* action = static_cast<jstring>(env->CallObjectMethod(intent, get_action));
    if (env->ExceptionCheck() || !string_equals(env, action, "android.intent.action.VIEW")) {
        clear_pending_exception(env);
        return nullptr;
    }

    jobject uri = env->CallObjectMethod(intent, get_data);
    if (uri == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return nullptr;
    }
    jclass uri_class = env->FindClass("android/net/Uri");
    jmethodID get_scheme = uri_class == nullptr
        ? nullptr
        : env->GetMethodID(uri_class, "getScheme", "()Ljava/lang/String;");
    jmethodID get_host = uri_class == nullptr
        ? nullptr
        : env->GetMethodID(uri_class, "getHost", "()Ljava/lang/String;");
    if (get_scheme == nullptr || get_host == nullptr) {
        clear_pending_exception(env);
        return nullptr;
    }

    auto* scheme = static_cast<jstring>(env->CallObjectMethod(uri, get_scheme));
    auto* host = static_cast<jstring>(env->CallObjectMethod(uri, get_host));
    if (env->ExceptionCheck()
            || !string_equals(env, scheme, "kakaolink")
            || !string_equals(env, host, "send")) {
        clear_pending_exception(env);
        return nullptr;
    }

    auto* data = static_cast<jstring>(env->CallObjectMethod(intent, get_data_string));
    if (data == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return nullptr;
    }
    if (env->GetStringLength(data) > kMaximumUriLength) {
        return nullptr;
    }
    return data;
}

bool get_boolean_preference(JNIEnv* env, jobject activity, const char* key) {
    jclass activity_class = env->GetObjectClass(activity);
    jmethodID get_preferences = activity_class == nullptr
        ? nullptr
        : env->GetMethodID(
              activity_class,
              "getSharedPreferences",
              "(Ljava/lang/String;I)Landroid/content/SharedPreferences;");
    if (get_preferences == nullptr) {
        clear_pending_exception(env);
        return false;
    }

    jstring name = env->NewStringUTF(kPreferencesName);
    jobject preferences = env->CallObjectMethod(activity, get_preferences, name, 0);
    if (preferences == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return false;
    }
    jclass preferences_class = env->FindClass("android/content/SharedPreferences");
    jmethodID get_boolean = preferences_class == nullptr
        ? nullptr
        : env->GetMethodID(preferences_class, "getBoolean", "(Ljava/lang/String;Z)Z");
    if (get_boolean == nullptr) {
        clear_pending_exception(env);
        return false;
    }

    jstring preference_key = env->NewStringUTF(key);
    const jboolean value = env->CallBooleanMethod(preferences, get_boolean, preference_key, JNI_FALSE);
    if (env->ExceptionCheck()) {
        clear_pending_exception(env);
        return false;
    }
    return value == JNI_TRUE;
}

jstring build_portable_url(JNIEnv* env, jstring deep_link) {
    jclass uri_class = env->FindClass("android/net/Uri");
    jmethodID encode = uri_class == nullptr
        ? nullptr
        : env->GetStaticMethodID(uri_class, "encode", "(Ljava/lang/String;)Ljava/lang/String;");
    if (encode == nullptr) {
        clear_pending_exception(env);
        return nullptr;
    }

    auto* encoded = static_cast<jstring>(env->CallStaticObjectMethod(uri_class, encode, deep_link));
    if (encoded == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return nullptr;
    }

    jclass string_class = env->FindClass("java/lang/String");
    jmethodID concat = string_class == nullptr
        ? nullptr
        : env->GetMethodID(string_class, "concat", "(Ljava/lang/String;)Ljava/lang/String;");
    if (concat == nullptr) {
        clear_pending_exception(env);
        return nullptr;
    }

    jstring prefix = env->NewStringUTF(kRedirectPrefix);
    auto* portable_url = static_cast<jstring>(env->CallObjectMethod(prefix, concat, encoded));
    if (env->ExceptionCheck()) {
        clear_pending_exception(env);
        return nullptr;
    }
    return portable_url;
}

bool mark_clip_sensitive(JNIEnv* env, jobject clip) {
    jclass clip_data_class = env->GetObjectClass(clip);
    jmethodID get_description = clip_data_class == nullptr
        ? nullptr
        : env->GetMethodID(
              clip_data_class, "getDescription", "()Landroid/content/ClipDescription;");
    if (get_description == nullptr) {
        clear_pending_exception(env);
        return false;
    }
    jobject description = env->CallObjectMethod(clip, get_description);
    if (description == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return false;
    }

    jclass bundle_class = env->FindClass("android/os/PersistableBundle");
    jmethodID bundle_constructor = bundle_class == nullptr
        ? nullptr
        : env->GetMethodID(bundle_class, "<init>", "()V");
    jmethodID put_boolean = bundle_class == nullptr
        ? nullptr
        : env->GetMethodID(bundle_class, "putBoolean", "(Ljava/lang/String;Z)V");
    if (bundle_constructor == nullptr || put_boolean == nullptr) {
        clear_pending_exception(env);
        return false;
    }
    jobject extras = env->NewObject(bundle_class, bundle_constructor);
    jstring key = env->NewStringUTF(kSensitiveExtra);
    env->CallVoidMethod(extras, put_boolean, key, JNI_TRUE);
    if (env->ExceptionCheck()) {
        clear_pending_exception(env);
        return false;
    }

    jclass description_class = env->FindClass("android/content/ClipDescription");
    jmethodID set_extras = description_class == nullptr
        ? nullptr
        : env->GetMethodID(
              description_class, "setExtras", "(Landroid/os/PersistableBundle;)V");
    if (set_extras == nullptr) {
        clear_pending_exception(env);
        return false;
    }
    env->CallVoidMethod(description, set_extras, extras);
    if (env->ExceptionCheck()) {
        clear_pending_exception(env);
        return false;
    }
    return true;
}

bool copy_to_clipboard(
        JNIEnv* env,
        jobject activity,
        jstring text,
        bool link_mode,
        bool sensitive) {
    jclass activity_class = env->GetObjectClass(activity);
    jmethodID get_system_service = activity_class == nullptr
        ? nullptr
        : env->GetMethodID(
              activity_class,
              "getSystemService",
              "(Ljava/lang/String;)Ljava/lang/Object;");
    if (get_system_service == nullptr) {
        clear_pending_exception(env);
        return false;
    }

    jstring service_name = env->NewStringUTF("clipboard");
    jobject clipboard = env->CallObjectMethod(activity, get_system_service, service_name);
    if (clipboard == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return false;
    }

    jclass clip_data_class = env->FindClass("android/content/ClipData");
    jmethodID new_plain_text = clip_data_class == nullptr
        ? nullptr
        : env->GetStaticMethodID(
              clip_data_class,
              "newPlainText",
              "(Ljava/lang/CharSequence;Ljava/lang/CharSequence;)Landroid/content/ClipData;");
    if (new_plain_text == nullptr) {
        clear_pending_exception(env);
        return false;
    }

    jstring label = env->NewStringUTF(link_mode ? "KakaoLink Pages link" : "KakaoLink data URI");
    jobject clip = env->CallStaticObjectMethod(clip_data_class, new_plain_text, label, text);
    if (clip == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return false;
    }
    if (sensitive && !mark_clip_sensitive(env, clip)) {
        return false;
    }

    jclass clipboard_class = env->GetObjectClass(clipboard);
    jmethodID set_primary_clip = clipboard_class == nullptr
        ? nullptr
        : env->GetMethodID(clipboard_class, "setPrimaryClip", "(Landroid/content/ClipData;)V");
    if (set_primary_clip == nullptr) {
        clear_pending_exception(env);
        return false;
    }

    env->CallVoidMethod(clipboard, set_primary_clip, clip);
    if (env->ExceptionCheck()) {
        clear_pending_exception(env);
        return false;
    }
    return true;
}

}  // namespace

extern "C" JNIEXPORT void ANativeActivity_onCreate(
    ANativeActivity* activity, void*, size_t) {
    JNIEnv* env = activity->env;
    jobject native_activity = activity->clazz;
    if (env == nullptr || native_activity == nullptr) {
        return;
    }

    jstring deep_link = get_valid_intent_data(env, native_activity);
    if (deep_link == nullptr) {
        finish(env, native_activity);
        return;
    }

    const bool link_mode = get_boolean_preference(env, native_activity, kUseLinkPreference);
    const bool sensitive = get_boolean_preference(env, native_activity, kSensitivePreference);
    jstring output = link_mode ? build_portable_url(env, deep_link) : deep_link;
    if (output == nullptr) {
        show_toast(env, native_activity, "KakaoLink Capture: encode failed");
        finish(env, native_activity);
        return;
    }

    if (copy_to_clipboard(env, native_activity, output, link_mode, sensitive)) {
        show_toast(
            env,
            native_activity,
            link_mode ? "GitHub Pages link copied" : "Original KakaoLink URI copied");
    } else {
        show_toast(env, native_activity, "KakaoLink Capture: clipboard failed");
    }
    finish(env, native_activity);
}
