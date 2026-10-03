#include "PluginEditor.h"
#include <BinaryData.h>

YuroExactProcessorEditor::YuroExactProcessorEditor(YuroExactProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      browser(juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
              .withUserDataFolder(juce::File::getSpecialLocation(juce::File::tempDirectory)
                                  .getChildFile("808YURO_WebView2")))
          .withNativeIntegrationEnabled(true)
          .withEventListener("808yuro_params", [this](const juce::var& v)
          {
              if (!v.isObject()) return;
              auto* o = v.getDynamicObject();
              if (o == nullptr) return;
              const int effect = juce::jlimit(0, 15, (int)o->getProperty("effect"));
              const int style  = juce::jlimit(0, 3, (int)o->getProperty("style"));
              const int grid   = juce::jlimit(0, 1, (int)o->getProperty("grid"));
              const double bpm = juce::jlimit(40.0, 240.0, (double)o->getProperty("bpm"));
              const float amount = juce::jlimit(0.0f, 1.0f, (float)o->getProperty("amount"));
              const int mode = juce::jlimit(0, 2, (int)o->getProperty("view"));

              if (auto* q = processor.apvts.getParameter("effect")) q->setValueNotifyingHost(effect / 15.0f);
              if (auto* q = processor.apvts.getParameter("style")) q->setValueNotifyingHost(style / 3.0f);
              if (auto* q = processor.apvts.getParameter("grid")) q->setValueNotifyingHost((float)grid);
              if (auto* q = processor.apvts.getParameter("mode")) q->setValueNotifyingHost(mode / 2.0f);
              if (auto* q = processor.apvts.getParameter("amount")) q->setValueNotifyingHost(amount);
              if (auto* q = processor.apvts.getParameter("bpm")) q->setValueNotifyingHost((float)((bpm - 40.0) / 200.0));
          })
          .withResourceProvider([](const juce::String& path) -> std::optional<juce::WebBrowserComponent::Resource>
          {
              if (path == "/" || path == "/index.html")
              {
                  juce::WebBrowserComponent::Resource r;
                  r.mimeType = "text/html";
                  const auto* begin = reinterpret_cast<const std::byte*>(BinaryData::index_html);
                  const auto* end = begin + BinaryData::index_htmlSize;
                  r.data.assign(begin, end);
                  return r;
              }
              return std::nullopt;
          }))
{
    setResizable(false, false);
    setSize(920, 650);
    addAndMakeVisible(browser);
    browser.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
}

void YuroExactProcessorEditor::resized()
{
    browser.setBounds(getLocalBounds());
}
