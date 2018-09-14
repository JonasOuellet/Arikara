#pragma once

#include <maya/MDoubleArray.h>

class InteractiveEdit;
class QSlider;
class ArikaraSkin;

class SkinModeBase
{
public:
    SkinModeBase(const char* modeName);

    const char* getModeName();

    virtual void init(InteractiveEdit&);
    virtual void edit(InteractiveEdit&);
    virtual void end(InteractiveEdit&);

    virtual void setWeight(double pVal);

    virtual void updateSlider(QSlider* pSlider, ArikaraSkin* pArikara);

    virtual bool needVertexData() { return true; }

private:
    const char* m_modeName;

};

class AbsoluteSkinMode : public SkinModeBase
{
public:
    AbsoluteSkinMode() : SkinModeBase("Absolute") {};

    static SkinModeBase* creator();
    //virtual void init(InteractiveEdit&) override;
    //virtual void edit(InteractiveEdit&) override;
};

class RelativeSkinMode : public SkinModeBase
{
public:
    RelativeSkinMode() : SkinModeBase("Relative") {};

    //virtual void init(InteractiveEdit&) override;
    virtual void edit(InteractiveEdit&) override;
    virtual void updateSlider(QSlider* pSlider, ArikaraSkin* pArikara) override;

    static SkinModeBase* creator();
};

class ScaleSkinMode : public SkinModeBase
{
public:
    ScaleSkinMode() : SkinModeBase("Scale") {};

    //virtual void init(InteractiveEdit&) override;
    virtual void edit(InteractiveEdit&) override;
    virtual void updateSlider(QSlider* pSlider, ArikaraSkin* pArikara) override;

    static SkinModeBase* creator();
};

class RigidSkinMode : public SkinModeBase
{
public:
    RigidSkinMode() : SkinModeBase("Rigid") {};

    virtual void init(InteractiveEdit&) override;
    virtual void edit(InteractiveEdit&) override;
    virtual void end(InteractiveEdit&) override;

    virtual void updateSlider(QSlider* pSlider, ArikaraSkin* pArikara) override;

    virtual bool needVertexData() override { return false; }

    static SkinModeBase* creator();

private:
    MDoubleArray m_TargetWeights;
};