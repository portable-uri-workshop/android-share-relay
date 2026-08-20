#include <android/native_activity.h>
#include <jni.h>

namespace {

constexpr char kRedirectPrefix[] =
    "https://deep-link-redirector.major-kite-5089.chatgpt.site/#to=";

void clear_pending_exception(JNIEnv* env) {
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
}
void show_toast(JNIEnv* env, jobject activity, const char* text) {
    jclass toast_class = env->FindClass("android/widget/Toast");
    if (toast_class == nullptr) {
        clear_pending_exception(env);
        return;
    }

    jmethodID make_text = env->GetStaticMethodID(
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

    jmethodID show = env->GetMethodID(toast_class, "show", "()V");
    if (show != nullptr) {
        env->CallVoidMethod(toast, show);
        clear_pending_exception(env);
    }
}

void finish(JNIEnv* env, jobject activity) {
    jclass activity_class = env->GetObjectClass(activity);
    if (activity_class == nullptr) {
        clear_pending_exception(env);
        return;
    }

    jmethodID finish_method = env->GetMethodID(activity_class, "finish", "()V");
    if (finish_method != nullptr) {
        env->CallVoidMethod(activity, finish_method);
        clear_pending_exception(env);
    }
}

jstring get_intent_data(JNIEnv* env, jobject activity) {
    jclass activity_class = env->GetObjectClass(activity);
    if (activity_class == nullptr) {
        clear_pending_exception(env);
        return nullptr;
    }

    jmethodID get_intent = env->GetMethodID(
        activity_class, "getIntent", "()Landroid/content/Intent;");
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
    jmethodID get_data_string = intent_class == nullptr
        ? nullptr
        : env->GetMethodID(intent_class, "getDataString", "()Ljava/lang/String;");
    if (get_data_string == nullptr) {
        clear_pending_exception(env);
        return nullptr;
    }

    auto* data = static_cast<jstring>(env->CallObjectMethod(intent, get_data_string));
    if (env->ExceptionCheck()) {
        clear_pending_exception(env);
        return nullptr;
    }
    return data;
}

jstring build_portable_url(JNIEnv* env, jstring deep_link) {
    jclass uri_class = env->FindClass("android/net/Uri");
    jmethodID encode = uri_class == nullptr
        ? nullptr
        : env->GetStaticMethodID(
              uri_class, "encode", "(Ljava/lang/String;)Ljava/lang/String;");
    if (encode == nullptr) {
        clear_pending_exception(env);
        return nullptr;
    }

    auto* encoded = static_cast<jstring>(
        env->CallStaticObjectMethod(uri_class, encode, deep_link));
    if (encoded == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return nullptr;
    }

    jclass string_class = env->FindClass("java/lang/String");
    jmethodID concat = string_class == nullptr
        ? nullptr
        : env->GetMethodID(
              string_class, "concat", "(Ljava/lang/String;)Ljava/lang/String;");
    if (concat == nullptr) {
        clear_pending_exception(env);
        return nullptr;
    }

    jstring prefix = env->NewStringUTF(kRedirectPrefix);
    auto* portable_url = static_cast<jstring>(
        env->CallObjectMethod(prefix, concat, encoded));
    if (env->ExceptionCheck()) {
        clear_pending_exception(env);
        return nullptr;
    }
    return portable_url;
}

bool copy_to_clipboard(JNIEnv* env, jobject activity, jstring text) {
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

    jstring label = env->NewStringUTF("KakaoLink redirect");
    jobject clip = env->CallStaticObjectMethod(
        clip_data_class, new_plain_text, label, text);
    if (clip == nullptr || env->ExceptionCheck()) {
        clear_pending_exception(env);
        return false;
    }

    jclass clipboard_class = env->GetObjectClass(clipboard);
    jmethodID set_primary_clip = clipboard_class == nullptr
        ? nullptr
        : env->GetMethodID(
              clipboard_class,
              "setPrimaryClip",
              "(Landroid/content/ClipData;)V");
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

    jstring deep_link = get_intent_data(env, native_activity);
    if (deep_link == nullptr) {
        show_toast(env, native_activity, "KakaoLink Capture ready");
        finish(env, native_activity);
        return;
    }

    jstring portable_url = build_portable_url(env, deep_link);
    if (portable_url == nullptr) {
        show_toast(env, native_activity, "KakaoLink Capture: encode failed");
        finish(env, native_activity);
        return;
    }

    if (copy_to_clipboard(env, native_activity, portable_url)) {
        show_toast(env, native_activity, "KakaoLink #to URL copied");
    } else {
        show_toast(env, native_activity, "KakaoLink Capture: clipboard failed");
    }
    finish(env, native_activity);
}
