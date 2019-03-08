using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;

public class imgui_integration_unity : MonoBehaviour {

#if UNITY_EDITOR
    [DllImport("imgui_integration_unity_Editor")]    
#else
    [DllImport("imgui_integration_unity")]    
#endif
    static extern IntPtr GetRenderEventFunc();

#if UNITY_EDITOR
    [DllImport("imgui_integration_unity_Editor")]
#else
    [DllImport("imgui_integration_unity")]
#endif
    public static extern void SetFacialWeights(int count, IntPtr weights);

    IEnumerator Start()
    {
        yield return StartCoroutine("CallNativePlugin");
    }


    IEnumerator CallNativePlugin()
    {
        while (true)
        {
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(GetRenderEventFunc(), 1);
        }
    }
}
