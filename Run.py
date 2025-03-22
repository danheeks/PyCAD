import subprocess

# Start the splash screen in a separate process, but don't wait before starting the app
splash_process = subprocess.Popen(['python', 'Splash.py'])

# Start the main application
main_process = subprocess.Popen(['python', 'SolidApp.py'])

# Optionally, wait for the main application to exit before closing the splash screen
main_process.wait()

# Once the main app is done, terminate the splash screen
splash_process.terminate()
