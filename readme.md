# Academia

**Transforming education through a unified digital ecosystem**

Academia is a school management system designed to bring grades, attendance, announcements, and student information together in one place. It aims to make it easier for students, parents, and school staff to find the information they need and stay connected.


## Why Academia?

Schools often rely on separate websites and tools for academic records and communication. Academia aims to provide a clearer, more convenient experience by giving each member of the school community one place to access relevant information.

## Planned features

### Students

- View grades and GPA.
- Check attendance.
- Read school announcements.

### Parents

- View their child's grades and attendance.
- Keep up with school announcements and notices.

### School staff

- Enter and manage grades.
- Record attendance.
- Publish announcements.

### Project goals

- **Role-based access:** Give users access to the information and tools appropriate to their responsibilities.
- **A reliable core first:** Complete and test essential features before expanding the scope.
- **Modular design:** Keep the system ready for future additions without making the core harder to maintain.
- **Accessible setup:** Aim to run without special hardware and provide sample data for learning and testing.

## Possible future enhancements

Schedules, course resources, parent-teacher messaging, weather updates, college and scholarship planning, advanced reporting, database support, and a graphical interface are ideas for future development—not current features.

## Technology and current repository

The current solution is built with **C# and .NET 10**:

- `SampleApp/FrontEnd` is a Blazor Server web application.
- `SampleApp/BackEnd` is an ASP.NET Core API with a sample weather forecast endpoint and development OpenAPI documentation.

The current app is still the framework starter scaffold; it does not yet store or manage Academia school data. The original project concept mentions C, file handling, and structures, but those are not the technologies in the current solution.

## Run locally

### Prerequisites

- The .NET 10 SDK.

### Start the backend

In a terminal from the repository root:

```powershell
dotnet run --project SampleApp\BackEnd
```

The backend's HTTP launch profile uses `http://localhost:8080`.

### Start the frontend

In a second terminal from the repository root:

```powershell
dotnet run --project SampleApp\FrontEnd
```

Open `http://localhost:8081`. The frontend's `WEATHER_URL` setting defaults to `http://localhost:8080`, so start the backend as well when using the sample weather page.

To build both projects:

```powershell
dotnet build SampleApp\SampleApp.sln
```

## Project structure

```text
SampleApp/
  BackEnd/    ASP.NET Core API
  FrontEnd/   Blazor Server application
  SampleApp.sln
images/       Project images
```

## Contributing

Work toward the planned features by starting with a small, tested core. Keep the implementation and documentation aligned, and update this README as features become available.
