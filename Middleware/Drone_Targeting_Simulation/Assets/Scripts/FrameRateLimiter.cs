using UnityEngine;

public class FrameRateLimiter : MonoBehaviour 
{
    //Self Reference
    public static FrameRateLimiter Instance { get; private set; }
	
	public int targetFrameRate = 20;
	
	private void Awake()
    {
        //This will only pass once at the beggining of the game 
        if (Instance == null)
        {
            //Self reference
            Instance = this;
            //Make this object persistent
            DontDestroyOnLoad(gameObject);
        }
        else
        {
            //Destroy duplicate instance created by changing Scene
            Destroy(gameObject);
        }
    }
 
    private void Start()
    {
        QualitySettings.vSyncCount = 0;
        Application.targetFrameRate = targetFrameRate;
    }
}