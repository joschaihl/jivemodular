# C++ Console Executable Makefile autogenerated by premake
# Don't edit this file! Instead edit `premake.lua` then rerun `make`

ifndef CONFIG
  CONFIG=Debug
endif

# if multiple archs are defined turn off automated dependency generation
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifeq ($(CONFIG),Debug)
  BINDIR := ../../../../bin
  LIBDIR := ../../../../bin
  OBJDIR := ../../../../bin/intermediate_linux/jostDebug
  OUTDIR := ../../../../bin
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "JUCE_USE_XSHM=1" -D "JUCE_ALSA=1" -D "JUCE_JACK=1" -D "JUCE_USE_VSTSDK_2_4=1" -D "JOST_USE_VST=1" -D "JOST_USE_LADSPA=1" -D "JOST_USE_DSSI=1" -D "JOST_USE_JACKBRIDGE=0" -D "DEBUG=1" -D "_DEBUG=1" -I "/usr/include" -I "/usr/include/freetype2" -I "../../../../juce" -I "../../../../juce/src" -I "../../../../juce/extras/audio plugins" -I "../../src" -I "../../../../vst/vstsdk2.4/public.sdk/source/vst2.x" -I "../../../../vst/vstsdk2.4" -I "../../../../vstsdk2.4/public.sdk/source/vst2.x" -I "../../../../vstsdk2.4" -I "../../vst/vstsdk2.4/public.sdk/source/vst2.x" -I "../../vst/vstsdk2.4" -I "../../vstsdk2.4/public.sdk/source/vst2.x" -I "../../vstsdk2.4" -I "/usr/include/vstsdk2.4/public.sdk/source/vst2.x" -I "/usr/include/vst/public.sdk/source/vst2.x"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -O0 -g -Wall
  CXXFLAGS += $(CFLAGS)
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"../../../../bin" -L"/usr/X11R6/lib/" -L"/usr/lib/" -lfreetype -lpthread -lrt -lX11 -lXext -lasound -ljuce_debug
  LDDEPS :=
  RESFLAGS := -D "LINUX=1" -D "JUCE_USE_XSHM=1" -D "JUCE_ALSA=1" -D "JUCE_JACK=1" -D "JUCE_USE_VSTSDK_2_4=1" -D "JOST_USE_VST=1" -D "JOST_USE_LADSPA=1" -D "JOST_USE_DSSI=1" -D "JOST_USE_JACKBRIDGE=0" -D "DEBUG=1" -D "_DEBUG=1" -I "/usr/include" -I "/usr/include/freetype2" -I "../../../../juce" -I "../../../../juce/src" -I "../../../../juce/extras/audio plugins" -I "../../src" -I "../../../../vst/vstsdk2.4/public.sdk/source/vst2.x" -I "../../../../vst/vstsdk2.4" -I "../../../../vstsdk2.4/public.sdk/source/vst2.x" -I "../../../../vstsdk2.4" -I "../../vst/vstsdk2.4/public.sdk/source/vst2.x" -I "../../vst/vstsdk2.4" -I "../../vstsdk2.4/public.sdk/source/vst2.x" -I "../../vstsdk2.4" -I "/usr/include/vstsdk2.4/public.sdk/source/vst2.x" -I "/usr/include/vst/public.sdk/source/vst2.x"
  TARGET := jost_debug
 BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := ../../../../bin
  LIBDIR := ../../../../bin
  OBJDIR := ../../../../bin/intermediate_linux/jostRelease
  OUTDIR := ../../../../bin
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "JUCE_USE_XSHM=1" -D "JUCE_ALSA=1" -D "JUCE_JACK=1" -D "JUCE_USE_VSTSDK_2_4=1" -D "JOST_USE_VST=1" -D "JOST_USE_LADSPA=1" -D "JOST_USE_DSSI=1" -D "JOST_USE_JACKBRIDGE=0" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2" -I "../../../../juce" -I "../../../../juce/src" -I "../../../../juce/extras/audio plugins" -I "../../src" -I "../../../../vst/vstsdk2.4/public.sdk/source/vst2.x" -I "../../../../vst/vstsdk2.4" -I "../../../../vstsdk2.4/public.sdk/source/vst2.x" -I "../../../../vstsdk2.4" -I "../../vst/vstsdk2.4/public.sdk/source/vst2.x" -I "../../vst/vstsdk2.4" -I "../../vstsdk2.4/public.sdk/source/vst2.x" -I "../../vstsdk2.4" -I "/usr/include/vstsdk2.4/public.sdk/source/vst2.x" -I "/usr/include/vst/public.sdk/source/vst2.x"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -O3 -fomit-frame-pointer -pipe -fvisibility=hidden -Wall
  CXXFLAGS += $(CFLAGS)
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -s -L"../../../../bin" -L"/usr/X11R6/lib/" -L"/usr/lib/" -lfreetype -lpthread -lrt -lX11 -lXext -lasound -ljuce
  LDDEPS :=
  RESFLAGS := -D "LINUX=1" -D "JUCE_USE_XSHM=1" -D "JUCE_ALSA=1" -D "JUCE_JACK=1" -D "JUCE_USE_VSTSDK_2_4=1" -D "JOST_USE_VST=1" -D "JOST_USE_LADSPA=1" -D "JOST_USE_DSSI=1" -D "JOST_USE_JACKBRIDGE=0" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2" -I "../../../../juce" -I "../../../../juce/src" -I "../../../../juce/extras/audio plugins" -I "../../src" -I "../../../../vst/vstsdk2.4/public.sdk/source/vst2.x" -I "../../../../vst/vstsdk2.4" -I "../../../../vstsdk2.4/public.sdk/source/vst2.x" -I "../../../../vstsdk2.4" -I "../../vst/vstsdk2.4/public.sdk/source/vst2.x" -I "../../vst/vstsdk2.4" -I "../../vstsdk2.4/public.sdk/source/vst2.x" -I "../../vstsdk2.4" -I "/usr/include/vstsdk2.4/public.sdk/source/vst2.x" -I "/usr/include/vst/public.sdk/source/vst2.x"
  TARGET := jost
 BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
	$(OBJDIR)/HostFilterBase.o \
	$(OBJDIR)/Config.o \
	$(OBJDIR)/StandardLibrary.o \
	$(OBJDIR)/HostFilterComponent.o \
	$(OBJDIR)/Commands.o \
	$(OBJDIR)/Main.o \
	$(OBJDIR)/Host.o \
	$(OBJDIR)/PluginLoader.o \
	$(OBJDIR)/MultiTrack.o \
	$(OBJDIR)/BasePlugin.o \
	$(OBJDIR)/Transport.o \
	$(OBJDIR)/InputPlugin.o \
	$(OBJDIR)/ChannelOutputPlugin.o \
	$(OBJDIR)/OutputPlugin.o \
	$(OBJDIR)/ChannelInputPlugin.o \
	$(OBJDIR)/LadspaPlugin.o \
	$(OBJDIR)/TrackPlugin.o \
	$(OBJDIR)/VstPlugin.o \
	$(OBJDIR)/DssiPlugin.o \
	$(OBJDIR)/DetunerPlugin.o \
	$(OBJDIR)/OppressorPlugin.o \
	$(OBJDIR)/OppressorEditor.o \
	$(OBJDIR)/DetunerEditor.o \
	$(OBJDIR)/OverdosePlugin.o \
	$(OBJDIR)/OverdoseEditor.o \
	$(OBJDIR)/AudioSpecMeterEditor.o \
	$(OBJDIR)/AudioSpecMeterPlugin.o \
	$(OBJDIR)/MidiPads.o \
	$(OBJDIR)/MidiPadsPluginEditor.o \
	$(OBJDIR)/MidiSequencePluginBase.o \
	$(OBJDIR)/MidiMonitorPlugin.o \
	$(OBJDIR)/MidiFilterEditor.o \
	$(OBJDIR)/MidiFilterPlugin.o \
	$(OBJDIR)/MidiKeyboardPlugin.o \
	$(OBJDIR)/MidiSequencePlugin.o \
	$(OBJDIR)/MidiOutputPlugin.o \
	$(OBJDIR)/SequenceComponent.o \
	$(OBJDIR)/MidiMonitorEditor.o \
	$(OBJDIR)/MidiInputPlugin.o \
	$(OBJDIR)/MidiKeyboardEditor.o \
	$(OBJDIR)/ChannelGraphComponent.o \
	$(OBJDIR)/ChannelEditor.o \
	$(OBJDIR)/ChannelPlugin.o \
	$(OBJDIR)/ChannelHost.o \
	$(OBJDIR)/Resources.o \
	$(OBJDIR)/MainTabbedComponent.o \
	$(OBJDIR)/TrackComponent.o \
	$(OBJDIR)/GraphComponent.o \
	$(OBJDIR)/AudioSequenceComponent.o \
	$(OBJDIR)/BrowserTabbedComponent.o \
	$(OBJDIR)/BookmarksComponent.o \
	$(OBJDIR)/DiskBrowserComponent.o \
	$(OBJDIR)/MixerStripComponent.o \
	$(OBJDIR)/MixerComponent.o \
	$(OBJDIR)/SurfaceProperties.o \
	$(OBJDIR)/SurfaceObjects.o \
	$(OBJDIR)/SurfaceComponent.o \
	$(OBJDIR)/PluginEditorComponent.o \
	$(OBJDIR)/VstPluginWindowTabPanel.o \
	$(OBJDIR)/VstPluginWindowContent.o \
	$(OBJDIR)/VstPluginExternalEditor.o \
	$(OBJDIR)/VstPluginWindow.o \
	$(OBJDIR)/VstPluginNativeEditor.o \
	$(OBJDIR)/ToolbarMainComponent.o \
	$(OBJDIR)/JostLookAndFeel.o \
	$(OBJDIR)/ColourScheme.o \

MKDIR_TYPE := msdos
CMD := $(subst \,\\,$(ComSpec)$(COMSPEC))
ifeq (,$(CMD))
  MKDIR_TYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(SHELL)))
  MKDIR_TYPE := posix
endif
ifeq ($(MKDIR_TYPE),posix)
  CMD_MKBINDIR := mkdir -p $(BINDIR)
  CMD_MKLIBDIR := mkdir -p $(LIBDIR)
  CMD_MKOUTDIR := mkdir -p $(OUTDIR)
  CMD_MKOBJDIR := mkdir -p $(OBJDIR)
else
  CMD_MKBINDIR := $(CMD) /c if not exist $(subst /,\\,$(BINDIR)) mkdir $(subst /,\\,$(BINDIR))
  CMD_MKLIBDIR := $(CMD) /c if not exist $(subst /,\\,$(LIBDIR)) mkdir $(subst /,\\,$(LIBDIR))
  CMD_MKOUTDIR := $(CMD) /c if not exist $(subst /,\\,$(OUTDIR)) mkdir $(subst /,\\,$(OUTDIR))
  CMD_MKOBJDIR := $(CMD) /c if not exist $(subst /,\\,$(OBJDIR)) mkdir $(subst /,\\,$(OBJDIR))
endif

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking jost
	-@$(CMD_MKBINDIR)
	-@$(CMD_MKLIBDIR)
	-@$(CMD_MKOUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning jost
ifeq ($(MKDIR_TYPE),posix)
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)
else
	-@if exist $(subst /,\,$(OUTDIR)/$(TARGET)) del /q $(subst /,\,$(OUTDIR)/$(TARGET))
	-@if exist $(subst /,\,$(OBJDIR)) del /q $(subst /,\,$(OBJDIR))
	-@if exist $(subst /,\,$(OBJDIR)) rmdir /s /q $(subst /,\,$(OBJDIR))
endif

$(OBJDIR)/HostFilterBase.o: ../../src/HostFilterBase.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Config.o: ../../src/Config.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/StandardLibrary.o: ../../src/StandardLibrary.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/HostFilterComponent.o: ../../src/HostFilterComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Commands.o: ../../src/Commands.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Main.o: ../../src/Main.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Host.o: ../../src/model/Host.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PluginLoader.o: ../../src/model/PluginLoader.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MultiTrack.o: ../../src/model/MultiTrack.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BasePlugin.o: ../../src/model/BasePlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Transport.o: ../../src/model/Transport.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/InputPlugin.o: ../../src/model/plugins/InputPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelOutputPlugin.o: ../../src/model/plugins/ChannelOutputPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OutputPlugin.o: ../../src/model/plugins/OutputPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelInputPlugin.o: ../../src/model/plugins/ChannelInputPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LadspaPlugin.o: ../../src/model/plugins/LadspaPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/TrackPlugin.o: ../../src/model/plugins/TrackPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/VstPlugin.o: ../../src/model/plugins/VstPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DssiPlugin.o: ../../src/model/plugins/DssiPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DetunerPlugin.o: ../../src/model/plugins/effects/DetunerPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OppressorPlugin.o: ../../src/model/plugins/effects/OppressorPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OppressorEditor.o: ../../src/model/plugins/effects/OppressorEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DetunerEditor.o: ../../src/model/plugins/effects/DetunerEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OverdosePlugin.o: ../../src/model/plugins/effects/OverdosePlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OverdoseEditor.o: ../../src/model/plugins/effects/OverdoseEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioSpecMeterEditor.o: ../../src/model/plugins/meters/AudioSpecMeterEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioSpecMeterPlugin.o: ../../src/model/plugins/meters/AudioSpecMeterPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiPads.o: ../../src/model/plugins/midiplugins/MidiPads.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiPadsPluginEditor.o: ../../src/model/plugins/midiplugins/MidiPadsPluginEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiSequencePluginBase.o: ../../src/model/plugins/midiplugins/MidiSequencePluginBase.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiMonitorPlugin.o: ../../src/model/plugins/midiplugins/MidiMonitorPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiFilterEditor.o: ../../src/model/plugins/midiplugins/MidiFilterEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiFilterPlugin.o: ../../src/model/plugins/midiplugins/MidiFilterPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiKeyboardPlugin.o: ../../src/model/plugins/midiplugins/MidiKeyboardPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiSequencePlugin.o: ../../src/model/plugins/midiplugins/MidiSequencePlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiOutputPlugin.o: ../../src/model/plugins/midiplugins/MidiOutputPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SequenceComponent.o: ../../src/model/plugins/midiplugins/SequenceComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiMonitorEditor.o: ../../src/model/plugins/midiplugins/MidiMonitorEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiInputPlugin.o: ../../src/model/plugins/midiplugins/MidiInputPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MidiKeyboardEditor.o: ../../src/model/plugins/midiplugins/MidiKeyboardEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelGraphComponent.o: ../../src/model/plugins/channelplugin/ChannelGraphComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelEditor.o: ../../src/model/plugins/channelplugin/ChannelEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelPlugin.o: ../../src/model/plugins/channelplugin/ChannelPlugin.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelHost.o: ../../src/model/plugins/channelplugin/ChannelHost.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Resources.o: ../../src/resources/Resources.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MainTabbedComponent.o: ../../src/ui/MainTabbedComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/TrackComponent.o: ../../src/ui/TrackComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/GraphComponent.o: ../../src/ui/GraphComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioSequenceComponent.o: ../../src/ui/AudioSequenceComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BrowserTabbedComponent.o: ../../src/ui/BrowserTabbedComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BookmarksComponent.o: ../../src/ui/browser/BookmarksComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DiskBrowserComponent.o: ../../src/ui/browser/DiskBrowserComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MixerStripComponent.o: ../../src/ui/mixer/MixerStripComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MixerComponent.o: ../../src/ui/mixer/MixerComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SurfaceProperties.o: ../../src/ui/surface/SurfaceProperties.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SurfaceObjects.o: ../../src/ui/surface/SurfaceObjects.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SurfaceComponent.o: ../../src/ui/surface/SurfaceComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PluginEditorComponent.o: ../../src/ui/plugins/PluginEditorComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/VstPluginWindowTabPanel.o: ../../src/ui/windows/VstPluginWindowTabPanel.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/VstPluginWindowContent.o: ../../src/ui/windows/VstPluginWindowContent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/VstPluginExternalEditor.o: ../../src/ui/windows/VstPluginExternalEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/VstPluginWindow.o: ../../src/ui/windows/VstPluginWindow.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/VstPluginNativeEditor.o: ../../src/ui/windows/VstPluginNativeEditor.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ToolbarMainComponent.o: ../../src/ui/toolbar/ToolbarMainComponent.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JostLookAndFeel.o: ../../src/ui/lookandfeel/JostLookAndFeel.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ColourScheme.o: ../../src/ui/lookandfeel/ColourScheme.cpp
	-@$(CMD_MKOBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)

