using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MovementController : MonoBehaviour
{
    public float acceleration = 60f; //Units per second^2
	public float topSpeed = 150f; //Units per second
    public float DegreesPerSecondRotation = 100f; //Degrees per second

    private Rigidbody rb;
    private float rotationDirection;
	
	private FrameRateLimiter FrameRate;

    void Start()
    {
        FrameRate = FrameRateLimiter.Instance;
		rb = GetComponent<Rigidbody>();
    }

    // Update is called once per frame
    void FixedUpdate()
    {
        if(FrameRate == null){
			Debug.LogError("Frame Rate not specified in FrameRateLimiter script", FrameRate);
		}
		
		Vector3 direction = new Vector3(0, Input.GetAxisRaw("Vertical") * acceleration / FrameRate.targetFrameRate, Input.GetKey(KeyCode.Space) ? acceleration / FrameRate.targetFrameRate : 0);
        direction = Quaternion.LookRotation(transform.forward) * direction;
        if (Vector3.Project(rb.velocity, direction).magnitude < topSpeed / FrameRate.targetFrameRate)
        {
            rb.AddForce(direction);
        }

        if (Input.GetAxisRaw("Horizontal") != 0)
        {
            transform.RotateAround(transform.position,  Mathf.Sign(Input.GetAxisRaw("Horizontal")) * Vector3.up, DegreesPerSecondRotation / FrameRate.targetFrameRate);
        }

    }

}
