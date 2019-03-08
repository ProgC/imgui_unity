using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;

public class RotateObj : MonoBehaviour {
    public imgui_integration_unity imgui;

	// Use this for initialization
	void Start () {
		
	}

    // Update is called once per frame
    float time = 0;
	void Update () {
        transform.Rotate(Vector3.up, 10 * Time.deltaTime, Space.World);

        time += Time.deltaTime;

        //if ( time > 2 )
        {
            time = 0;
                        
            float[] rot = new float[100];            

            for ( int i = 0; i < 30; ++i )
            {
                rot[i] = Random.Range(0, 100);
            }
                                   
            GCHandle gcRot = GCHandle.Alloc(rot, GCHandleType.Pinned);
            imgui_integration_unity.SetFacialWeights(30, gcRot.AddrOfPinnedObject());
            gcRot.Free();
        }        
	}
}
